#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace {

struct Rect {
    float x;
    float y;
    float width;
    float height;
};

struct Rgb {
    float red;
    float green;
    float blue;
};

struct App {
    std::string_view name;
    std::string_view description;
    Rgb accent;
};

struct AppCell {
    Rect bounds;
    Rect icon;
};

struct Layout {
    float scale;
    float margin;
    float gridTop;
    float rowHeight;
    float cellWidth;
    Rect dock;
    std::array<AppCell, 24> cells;
    std::array<Rect, 4> dockIcons;
};

constexpr std::array<App, 24> Apps{{
    {"ARCADE", "GAMES / COLLECTION", {0.98f, 0.42f, 0.35f}},
    {"ATLAS", "PLACES / NOTES", {0.35f, 0.78f, 0.72f}},
    {"AUDIO", "SOUND / LIBRARY", {0.82f, 0.68f, 0.97f}},
    {"BEATS", "RHYTHM / STUDIO", {0.98f, 0.67f, 0.30f}},
    {"BLOOM", "GROW / JOURNAL", {0.78f, 0.86f, 0.39f}},
    {"BOOKS", "READ / SAVED", {0.42f, 0.65f, 0.95f}},
    {"CAMERA", "IMAGE / CAPTURE", {0.95f, 0.53f, 0.68f}},
    {"CANVAS", "DRAW / IDEAS", {0.42f, 0.82f, 0.91f}},
    {"CLOCK", "TIME / TRACKER", {0.95f, 0.77f, 0.39f}},
    {"CODE", "BUILD / SCRATCHPAD", {0.55f, 0.82f, 0.54f}},
    {"COLOR", "PALETTE / LAB", {0.94f, 0.48f, 0.43f}},
    {"DRAFT", "WORDS / INBOX", {0.61f, 0.66f, 0.97f}},
    {"FILES", "ARCHIVE / INDEX", {0.91f, 0.72f, 0.39f}},
    {"FOCUS", "BREATH / RESET", {0.44f, 0.82f, 0.76f}},
    {"GAMES", "PLAY / RECENT", {0.91f, 0.50f, 0.58f}},
    {"GARDEN", "PLANTS / LOG", {0.54f, 0.79f, 0.42f}},
    {"JOURNAL", "DAILY / ENTRIES", {0.93f, 0.69f, 0.47f}},
    {"MAPS", "ROUTES / TRACES", {0.42f, 0.72f, 0.92f}},
    {"NOTES", "THOUGHTS / QUICK", {0.87f, 0.82f, 0.46f}},
    {"ORBIT", "MOTION / WATCH", {0.64f, 0.61f, 0.96f}},
    {"PHOTO", "STILLS / ALBUM", {0.95f, 0.54f, 0.41f}},
    {"PULSE", "HEALTH / SIGNAL", {0.90f, 0.43f, 0.52f}},
    {"RECORD", "VOICE / CLIPS", {0.67f, 0.77f, 0.93f}},
    {"STUDIO", "MAKE / PROJECTS", {0.56f, 0.83f, 0.68f}},
}};

constexpr std::array<std::size_t, 4> DockApps{{11, 2, 17, 23}};

glm::vec4 rgba(float red, float green, float blue, float alpha = 1.0f) {
    return {red, green, blue, alpha};
}

glm::vec4 rgba(Rgb value, float alpha = 1.0f) {
    return {value.red, value.green, value.blue, alpha};
}

float clamp(float value, float low, float high) {
    return std::max(low, std::min(value, high));
}

bool contains(const Rect& rect, float x, float y) {
    return x >= rect.x && y >= rect.y && x <= rect.x + rect.width && y <= rect.y + rect.height;
}

Layout makeLayout(float width, float height) {
    Layout layout{};
    layout.scale = clamp(std::min(width / 480.0f, height / 760.0f), 0.65f, 1.7f);
    layout.margin = std::max(18.0f, width * 0.055f);
    layout.gridTop = height * 0.112f;

    const float dockHeight = clamp(height * 0.105f, 64.0f, 92.0f);
    layout.dock = {
        layout.margin * 0.72f,
        height - dockHeight - height * 0.018f,
        width - layout.margin * 1.44f,
        dockHeight,
    };
    const float pageDotsY = layout.dock.y - std::max(14.0f, height * 0.035f);
    const float gridBottom = pageDotsY - height * 0.035f;
    const float columnGap = std::max(6.0f, width * 0.018f);
    const float rowGap = std::max(5.0f, height * 0.015f);
    layout.cellWidth = (width - 2.0f * layout.margin - 3.0f * columnGap) / 4.0f;
    layout.rowHeight = (gridBottom - layout.gridTop - 5.0f * rowGap) / 6.0f;
    const float iconSize = std::max(18.0f, std::min({layout.cellWidth * 0.72f, layout.rowHeight * 0.60f, std::min(width, height) * 0.15f}));

    for (std::size_t index = 0; index < layout.cells.size(); ++index) {
        const int row = static_cast<int>(index / 4);
        const int column = static_cast<int>(index % 4);
        const float cellX = layout.margin + column * (layout.cellWidth + columnGap);
        const float cellY = layout.gridTop + row * (layout.rowHeight + rowGap);
        const float iconX = cellX + (layout.cellWidth - iconSize) * 0.5f;
        const float iconY = cellY + std::max(0.0f, (layout.rowHeight - iconSize - 17.0f * layout.scale) * 0.5f);
        layout.cells[index] = {
            {cellX, cellY, layout.cellWidth, layout.rowHeight},
            {iconX, iconY, iconSize, iconSize},
        };
    }

    const float dockIconSize = std::min(layout.dock.height * 0.60f, layout.dock.width / 4.0f * 0.56f);
    for (std::size_t index = 0; index < layout.dockIcons.size(); ++index) {
        const float centerX = layout.dock.x + layout.dock.width * (static_cast<float>(index) + 0.5f) / 4.0f;
        const float centerY = layout.dock.y + layout.dock.height * 0.5f;
        layout.dockIcons[index] = {centerX - dockIconSize * 0.5f, centerY - dockIconSize * 0.5f, dockIconSize, dockIconSize};
    }
    return layout;
}

GLuint compileShader(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        return shader;
    }

    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(std::max(logLength, 1)), '\0');
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    glDeleteShader(shader);
    throw std::runtime_error("Shader compilation failed: " + log);
}

class Drawer {
public:
    Drawer() {
        constexpr const char* vertexSource = R"GLSL(
            #version 430 core
            layout (location = 0) in vec2 aPosition;
            uniform mat4 uProjection;
            uniform vec4 uRect;
            uniform vec4 uUvRect;
            out vec2 vLocal;
            out vec2 vUv;
            void main() {
                vec2 point = uRect.xy + aPosition * uRect.zw;
                gl_Position = uProjection * vec4(point, 0.0, 1.0);
                vLocal = aPosition * uRect.zw;
                vUv = uUvRect.xy + aPosition * uUvRect.zw;
            }
        )GLSL";
        constexpr const char* fragmentSource = R"GLSL(
            #version 430 core
            in vec2 vLocal;
            in vec2 vUv;
            uniform vec4 uColor;
            uniform vec2 uSize;
            uniform float uRadius;
            uniform int uTextured;
            uniform sampler2D uAtlas;
            out vec4 fragmentColor;
            void main() {
                if (uTextured == 1) {
                    float coverage = texture(uAtlas, vUv).r;
                    fragmentColor = vec4(uColor.rgb, uColor.a * coverage);
                } else {
                    vec2 point = abs(vLocal - uSize * 0.5) - (uSize * 0.5 - vec2(uRadius));
                    float distanceToEdge = length(max(point, 0.0)) + min(max(point.x, point.y), 0.0) - uRadius;
                    float edge = max(fwidth(distanceToEdge), 0.5);
                    float coverage = 1.0 - smoothstep(-edge, edge, distanceToEdge);
                    fragmentColor = vec4(uColor.rgb, uColor.a * coverage);
                }
            }
        )GLSL";

        const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
        const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        program_ = glCreateProgram();
        glAttachShader(program_, vertex);
        glAttachShader(program_, fragment);
        glLinkProgram(program_);
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        GLint linked = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE) {
            GLint logLength = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logLength);
            std::string log(static_cast<std::size_t>(std::max(logLength, 1)), '\0');
            glGetProgramInfoLog(program_, logLength, nullptr, log.data());
            throw std::runtime_error("Shader link failed: " + log);
        }

        projectionLocation_ = glGetUniformLocation(program_, "uProjection");
        rectLocation_ = glGetUniformLocation(program_, "uRect");
        uvRectLocation_ = glGetUniformLocation(program_, "uUvRect");
        colorLocation_ = glGetUniformLocation(program_, "uColor");
        sizeLocation_ = glGetUniformLocation(program_, "uSize");
        radiusLocation_ = glGetUniformLocation(program_, "uRadius");
        texturedLocation_ = glGetUniformLocation(program_, "uTextured");

        constexpr std::array<float, 12> vertices{
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        };
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
        glBindVertexArray(0);
    }

    ~Drawer() {
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
        glDeleteProgram(program_);
    }

    void setProjection(float width, float height) const {
        const glm::mat4 projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
        glUseProgram(program_);
        glUniformMatrix4fv(projectionLocation_, 1, GL_FALSE, &projection[0][0]);
        glUniform1i(glGetUniformLocation(program_, "uAtlas"), 0);
    }

    void roundedRect(const Rect& rect, glm::vec4 color, float radius) const {
        draw(rect, color, radius, false, {});
    }

    void glyph(const Rect& rect, glm::vec4 color, const glm::vec4& uv) const {
        draw(rect, color, 0.0f, true, uv);
    }

private:
    void draw(const Rect& rect, glm::vec4 color, float radius, bool textured, const glm::vec4& uv) const {
        if (rect.width <= 0.0f || rect.height <= 0.0f || color.a <= 0.0f) {
            return;
        }
        glUseProgram(program_);
        glUniform4f(rectLocation_, rect.x, rect.y, rect.width, rect.height);
        glUniform4fv(uvRectLocation_, 1, &uv[0]);
        glUniform4fv(colorLocation_, 1, &color[0]);
        glUniform2f(sizeLocation_, rect.width, rect.height);
        glUniform1f(radiusLocation_, std::min(radius, std::min(rect.width, rect.height) * 0.5f));
        glUniform1i(texturedLocation_, textured ? 1 : 0);
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    GLuint program_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLint projectionLocation_ = -1;
    GLint rectLocation_ = -1;
    GLint uvRectLocation_ = -1;
    GLint colorLocation_ = -1;
    GLint sizeLocation_ = -1;
    GLint radiusLocation_ = -1;
    GLint texturedLocation_ = -1;
};

struct Glyph {
    glm::vec4 uv{};
    int width = 0;
    int height = 0;
    int bearingX = 0;
    int bearingY = 0;
    int advance = 0;
};

class FontAtlas {
public:
    FontAtlas() {
        if (FT_Init_FreeType(&library_) != 0) {
            throw std::runtime_error("Could not initialize FreeType");
        }

        const std::array<const char*, 2> paths{
            "/usr/share/fonts/truetype/quicksand/Quicksand-Medium.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        };
        const auto fontPath = std::find_if(paths.begin(), paths.end(), [](const char* path) {
            return std::filesystem::exists(path);
        });
        if (fontPath == paths.end() || FT_New_Face(library_, *fontPath, 0, &face_) != 0) {
            FT_Done_FreeType(library_);
            throw std::runtime_error("No supported font was found under /usr/share/fonts");
        }

        constexpr int pixelSize = 48;
        constexpr int cellSize = 64;
        constexpr int columns = 16;
        constexpr int atlasWidth = columns * cellSize;
        constexpr int atlasHeight = 6 * cellSize;
        FT_Set_Pixel_Sizes(face_, 0, pixelSize);

        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (int character = 32; character <= 126; ++character) {
            if (FT_Load_Char(face_, static_cast<FT_ULong>(character), FT_LOAD_RENDER) != 0) {
                continue;
            }
            const FT_GlyphSlot slot = face_->glyph;
            const int index = character - 32;
            const int slotX = (index % columns) * cellSize;
            const int slotY = (index / columns) * cellSize;
            const int glyphWidth = static_cast<int>(slot->bitmap.width);
            const int glyphHeight = static_cast<int>(slot->bitmap.rows);
            if (glyphWidth > cellSize || glyphHeight > cellSize || glyphWidth == 0 || glyphHeight == 0) {
                glyphs_[static_cast<std::size_t>(character)] = {
                    {}, glyphWidth, glyphHeight, slot->bitmap_left, slot->bitmap_top,
                    static_cast<int>(slot->advance.x >> 6),
                };
                continue;
            }

            std::vector<unsigned char> pixels(static_cast<std::size_t>(glyphWidth * glyphHeight));
            const int pitch = std::abs(slot->bitmap.pitch);
            for (int row = 0; row < glyphHeight; ++row) {
                const int sourceRow = slot->bitmap.pitch >= 0 ? row : glyphHeight - 1 - row;
                std::memcpy(
                    pixels.data() + row * glyphWidth,
                    slot->bitmap.buffer + sourceRow * pitch,
                    static_cast<std::size_t>(glyphWidth));
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, slotX, slotY, glyphWidth, glyphHeight, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
            glyphs_[static_cast<std::size_t>(character)] = {
                glm::vec4(
                    static_cast<float>(slotX) / atlasWidth,
                    static_cast<float>(slotY) / atlasHeight,
                    static_cast<float>(glyphWidth) / atlasWidth,
                    static_cast<float>(glyphHeight) / atlasHeight),
                glyphWidth,
                glyphHeight,
                slot->bitmap_left,
                slot->bitmap_top,
                static_cast<int>(slot->advance.x >> 6),
            };
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        FT_Done_Face(face_);
        face_ = nullptr;
        FT_Done_FreeType(library_);
        library_ = nullptr;
    }

    ~FontAtlas() {
        glDeleteTextures(1, &texture_);
    }

    float measure(std::string_view text, float size) const {
        const float scale = size / 48.0f;
        float width = 0.0f;
        for (const unsigned char character : text) {
            const unsigned char index = character < glyphs_.size() ? character : static_cast<unsigned char>('?');
            width += glyphs_[index].advance * scale;
        }
        return width;
    }

    void draw(Drawer& drawer, std::string_view text, float x, float top, float size, glm::vec4 color, bool centered = false) const {
        const float scale = size / 48.0f;
        if (centered) {
            x -= measure(text, size) * 0.5f;
        }
        float cursor = x;
        const float baseline = top + size;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);

        for (const unsigned char character : text) {
            const unsigned char index = character < glyphs_.size() ? character : static_cast<unsigned char>('?');
            const Glyph& glyph = glyphs_[index];
            if (glyph.width > 0 && glyph.height > 0) {
                drawer.glyph(
                    {cursor + glyph.bearingX * scale, baseline - glyph.bearingY * scale,
                     glyph.width * scale, glyph.height * scale},
                    color,
                    glyph.uv);
            }
            cursor += glyph.advance * scale;
        }
    }

private:
    FT_Library library_ = nullptr;
    FT_Face face_ = nullptr;
    GLuint texture_ = 0;
    std::array<Glyph, 128> glyphs_{};
};

void drawCircle(Drawer& drawer, float centerX, float centerY, float radius, glm::vec4 color) {
    drawer.roundedRect({centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f}, color, radius);
}

void drawIcon(Drawer& drawer, const App& app, std::size_t index, const Rect& rect, float alpha) {
    const float x = rect.x;
    const float y = rect.y;
    const float size = rect.width;
    const glm::vec4 accent = rgba(app.accent, alpha);
    const glm::vec4 ink = rgba(0.055f, 0.075f, 0.075f, alpha);
    const glm::vec4 cutout = rgba(app.accent.red * 0.73f, app.accent.green * 0.73f, app.accent.blue * 0.73f, alpha);
    drawer.roundedRect(rect, accent, size * 0.27f);

    switch (index) {
    case 0:
        drawer.roundedRect({x + size * 0.43f, y + size * 0.23f, size * 0.14f, size * 0.54f}, ink, size * 0.05f);
        drawer.roundedRect({x + size * 0.23f, y + size * 0.43f, size * 0.54f, size * 0.14f}, ink, size * 0.05f);
        drawCircle(drawer, x + size * 0.76f, y + size * 0.28f, size * 0.055f, ink);
        drawCircle(drawer, x + size * 0.76f, y + size * 0.70f, size * 0.055f, ink);
        break;
    case 1:
    case 17:
        drawer.roundedRect({x + size * 0.24f, y + size * 0.22f, size * 0.52f, size * 0.56f}, ink, size * 0.05f);
        drawer.roundedRect({x + size * 0.40f, y + size * 0.22f, size * 0.06f, size * 0.56f}, accent, size * 0.03f);
        drawer.roundedRect({x + size * 0.59f, y + size * 0.22f, size * 0.06f, size * 0.56f}, accent, size * 0.03f);
        drawCircle(drawer, x + size * (index == 1 ? 0.52f : 0.64f), y + size * 0.50f, size * 0.09f, cutout);
        break;
    case 2:
        drawCircle(drawer, x + size * 0.5f, y + size * 0.5f, size * 0.27f, ink);
        drawCircle(drawer, x + size * 0.5f, y + size * 0.5f, size * 0.11f, accent);
        drawer.roundedRect({x + size * 0.46f, y + size * 0.15f, size * 0.08f, size * 0.18f}, ink, size * 0.04f);
        break;
    case 3:
    case 21:
        for (int bar = 0; bar < 5; ++bar) {
            const float amount = 0.17f + 0.10f * static_cast<float>((bar * (index == 3 ? 3 : 2) + 1) % 5);
            const float barHeight = size * amount;
            drawer.roundedRect({x + size * (0.18f + bar * 0.14f), y + (size - barHeight) * 0.5f, size * 0.08f, barHeight}, ink, size * 0.04f);
        }
        break;
    case 4:
        for (int petal = 0; petal < 5; ++petal) {
            const float angle = -1.5708f + petal * 1.2566f;
            drawCircle(drawer, x + size * (0.5f + std::cos(angle) * 0.20f), y + size * (0.5f + std::sin(angle) * 0.20f), size * 0.12f, ink);
        }
        drawCircle(drawer, x + size * 0.5f, y + size * 0.5f, size * 0.075f, cutout);
        break;
    case 5:
        for (int book = 0; book < 3; ++book) {
            const float bookX = x + size * (0.20f + book * 0.20f);
            drawer.roundedRect({bookX, y + size * (0.24f + (book % 2) * 0.07f), size * 0.17f, size * 0.53f}, ink, size * 0.035f);
            drawer.roundedRect({bookX + size * 0.04f, y + size * 0.34f, size * 0.025f, size * 0.31f}, accent, size * 0.012f);
        }
        break;
    case 6:
        drawer.roundedRect({x + size * 0.18f, y + size * 0.32f, size * 0.64f, size * 0.43f}, ink, size * 0.10f);
        drawer.roundedRect({x + size * 0.33f, y + size * 0.24f, size * 0.24f, size * 0.12f}, ink, size * 0.04f);
        drawCircle(drawer, x + size * 0.50f, y + size * 0.53f, size * 0.16f, accent);
        drawCircle(drawer, x + size * 0.50f, y + size * 0.53f, size * 0.09f, ink);
        drawCircle(drawer, x + size * 0.72f, y + size * 0.39f, size * 0.035f, accent);
        break;
    case 7:
        drawCircle(drawer, x + size * 0.39f, y + size * 0.39f, size * 0.19f, ink);
        drawCircle(drawer, x + size * 0.62f, y + size * 0.40f, size * 0.14f, cutout);
        drawer.roundedRect({x + size * 0.35f, y + size * 0.61f, size * 0.38f, size * 0.10f}, ink, size * 0.04f);
        break;
    case 8:
    case 13:
    case 19:
        drawCircle(drawer, x + size * 0.5f, y + size * 0.5f, size * 0.30f, ink);
        drawCircle(drawer, x + size * 0.5f, y + size * 0.5f, size * 0.23f, accent);
        if (index == 8) {
            drawer.roundedRect({x + size * 0.47f, y + size * 0.29f, size * 0.06f, size * 0.23f}, ink, size * 0.03f);
            drawer.roundedRect({x + size * 0.50f, y + size * 0.48f, size * 0.17f, size * 0.06f}, ink, size * 0.03f);
        } else if (index == 13) {
            drawer.roundedRect({x + size * 0.47f, y + size * 0.18f, size * 0.06f, size * 0.64f}, ink, size * 0.03f);
            drawer.roundedRect({x + size * 0.18f, y + size * 0.47f, size * 0.64f, size * 0.06f}, ink, size * 0.03f);
            drawCircle(drawer, x + size * 0.5f, y + size * 0.5f, size * 0.10f, cutout);
        } else {
            drawCircle(drawer, x + size * 0.61f, y + size * 0.39f, size * 0.08f, ink);
            drawCircle(drawer, x + size * 0.39f, y + size * 0.61f, size * 0.05f, ink);
        }
        break;
    case 9:
        drawer.roundedRect({x + size * 0.22f, y + size * 0.26f, size * 0.23f, size * 0.13f}, ink, size * 0.04f);
        drawer.roundedRect({x + size * 0.55f, y + size * 0.26f, size * 0.23f, size * 0.13f}, ink, size * 0.04f);
        drawer.roundedRect({x + size * 0.22f, y + size * 0.53f, size * 0.56f, size * 0.13f}, ink, size * 0.04f);
        break;
    case 10:
    case 14:
        for (int dot = 0; dot < 6; ++dot) {
            const float dotX = x + size * (0.28f + (dot % 3) * 0.22f);
            const float dotY = y + size * (0.32f + (dot / 3) * 0.25f);
            drawCircle(drawer, dotX, dotY, size * (index == 10 ? 0.075f : 0.05f), ink);
        }
        break;
    case 11:
    case 16:
    case 18:
        drawer.roundedRect({x + size * 0.26f, y + size * 0.18f, size * 0.48f, size * 0.64f}, ink, size * 0.06f);
        for (int row = 0; row < 3; ++row) {
            const float startX = index == 18 ? 0.42f : 0.38f;
            const float lineWidth = index == 16 && row == 2 ? 0.25f : 0.30f;
            drawer.roundedRect({x + size * startX, y + size * (0.36f + row * 0.13f), size * lineWidth, size * 0.035f}, accent, size * 0.017f);
            if (index == 18) {
                drawCircle(drawer, x + size * 0.33f, y + size * (0.38f + row * 0.13f), size * 0.025f, accent);
            }
        }
        break;
    case 12:
        drawer.roundedRect({x + size * 0.23f, y + size * 0.34f, size * 0.56f, size * 0.42f}, ink, size * 0.08f);
        drawer.roundedRect({x + size * 0.29f, y + size * 0.24f, size * 0.24f, size * 0.14f}, ink, size * 0.05f);
        drawer.roundedRect({x + size * 0.31f, y + size * 0.47f, size * 0.40f, size * 0.05f}, accent, size * 0.02f);
        break;
    case 15:
        drawer.roundedRect({x + size * 0.25f, y + size * 0.54f, size * 0.50f, size * 0.22f}, ink, size * 0.05f);
        drawer.roundedRect({x + size * 0.30f, y + size * 0.80f, size * 0.40f, size * 0.06f}, ink, size * 0.03f);
        drawCircle(drawer, x + size * 0.39f, y + size * 0.34f, size * 0.12f, ink);
        drawCircle(drawer, x + size * 0.61f, y + size * 0.34f, size * 0.12f, cutout);
        break;
    case 20:
        drawer.roundedRect({x + size * 0.20f, y + size * 0.24f, size * 0.60f, size * 0.52f}, ink, size * 0.06f);
        drawCircle(drawer, x + size * 0.63f, y + size * 0.39f, size * 0.07f, accent);
        drawer.roundedRect({x + size * 0.29f, y + size * 0.57f, size * 0.18f, size * 0.10f}, accent, size * 0.05f);
        drawer.roundedRect({x + size * 0.44f, y + size * 0.52f, size * 0.26f, size * 0.15f}, cutout, size * 0.06f);
        break;
    case 22:
        drawer.roundedRect({x + size * 0.40f, y + size * 0.20f, size * 0.20f, size * 0.44f}, ink, size * 0.10f);
        drawer.roundedRect({x + size * 0.30f, y + size * 0.46f, size * 0.40f, size * 0.10f}, ink, size * 0.05f);
        drawer.roundedRect({x + size * 0.47f, y + size * 0.57f, size * 0.06f, size * 0.18f}, ink, size * 0.03f);
        drawer.roundedRect({x + size * 0.36f, y + size * 0.75f, size * 0.28f, size * 0.06f}, ink, size * 0.03f);
        break;
    case 23:
        for (int slider = 0; slider < 3; ++slider) {
            const float sliderX = x + size * (0.30f + slider * 0.20f);
            drawer.roundedRect({sliderX, y + size * 0.23f, size * 0.07f, size * 0.54f}, ink, size * 0.035f);
            drawCircle(drawer, sliderX + size * 0.035f, y + size * (0.38f + slider * 0.13f), size * 0.10f, cutout);
        }
        break;
    }
}

void drawHome(Drawer& drawer, const FontAtlas& font, const Layout& layout, float width, float height, int selectedIndex, float openProgress, float introTime) {
    const glm::vec4 white = rgba(0.93f, 0.94f, 0.92f);
    const glm::vec4 muted = rgba(0.62f, 0.64f, 0.64f);
    const float scale = layout.scale;
    const float homeAlpha = 1.0f - clamp(openProgress * 1.35f, 0.0f, 1.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    font.draw(drawer, "10:28", layout.margin, height * 0.025f, 12.0f * scale, rgba(white.r, white.g, white.b, homeAlpha));

    const float statusRight = width - layout.margin;
    for (int bar = 0; bar < 4; ++bar) {
        const float barHeight = (5.0f + bar * 2.2f) * scale;
        drawer.roundedRect({statusRight - 59.0f * scale + bar * 5.0f * scale, height * 0.045f - barHeight, 3.0f * scale, barHeight}, rgba(muted.r, muted.g, muted.b, homeAlpha), 1.5f * scale);
    }
    drawer.roundedRect({statusRight - 33.0f * scale, height * 0.024f, 20.0f * scale, 10.0f * scale}, rgba(0.16f, 0.18f, 0.18f, homeAlpha), 4.0f * scale);
    drawer.roundedRect({statusRight - 31.0f * scale, height * 0.024f + 2.0f * scale, 14.0f * scale, 6.0f * scale}, rgba(0.77f, 0.84f, 0.65f, homeAlpha), 3.0f * scale);
    drawer.roundedRect({statusRight - 12.0f * scale, height * 0.027f, 2.0f * scale, 4.0f * scale}, rgba(muted.r, muted.g, muted.b, homeAlpha), 1.0f * scale);

    const float gridOffset = -openProgress * 14.0f * scale;
    for (std::size_t index = 0; index < layout.cells.size(); ++index) {
        const AppCell& cell = layout.cells[index];
        const int row = static_cast<int>(index / 4);
        const int column = static_cast<int>(index % 4);
        const float delay = row * 0.028f + (column % 2) * 0.014f;
        const float entrance = clamp((introTime - delay) / 0.38f, 0.0f, 1.0f);
        const float easedEntrance = 1.0f - std::pow(1.0f - entrance, 3.0f);
        const float iconScale = (0.78f + 0.22f * easedEntrance) * (1.0f - 0.045f * openProgress);
        const float iconWidth = cell.icon.width * iconScale;
        const float iconHeight = cell.icon.height * iconScale;
        const Rect iconRect{
            cell.icon.x + (cell.icon.width - iconWidth) * 0.5f,
            cell.icon.y + (cell.icon.height - iconHeight) * 0.5f + gridOffset,
            iconWidth,
            iconHeight,
        };
        const float alpha = easedEntrance * homeAlpha;
        drawIcon(drawer, Apps[index], index, iconRect, alpha);
        const float labelSize = clamp(layout.rowHeight * 0.145f * scale, 8.0f, 13.0f);
        const glm::vec4 labelColor = static_cast<int>(index) == selectedIndex ? rgba(0.88f, 0.90f, 0.84f, alpha) : rgba(0.68f, 0.70f, 0.70f, alpha);
        font.draw(drawer, Apps[index].name, cell.bounds.x + cell.bounds.width * 0.5f, cell.icon.y + cell.icon.height + 5.0f * scale + gridOffset, labelSize, labelColor, true);
    }

    const float pageY = layout.dock.y - std::max(14.0f, height * 0.035f) + openProgress * 16.0f * scale;
    for (int page = 0; page < 3; ++page) {
        const float centerX = width * 0.5f + (page - 1) * 12.0f * scale;
        const float dotWidth = page == 0 ? 13.0f * scale : 4.0f * scale;
        drawer.roundedRect({centerX - dotWidth * 0.5f, pageY - 2.0f * scale, dotWidth, 4.0f * scale},
                           rgba(0.74f, 0.76f, 0.75f, homeAlpha), 2.0f * scale);
    }

    const float dockEntrance = clamp(introTime / 0.48f, 0.0f, 1.0f);
    const float dockAlpha = (1.0f - openProgress) * dockEntrance;
    Rect dock = layout.dock;
    dock.y += (1.0f - dockEntrance) * 22.0f * scale + openProgress * 28.0f * scale;
    drawer.roundedRect(dock, rgba(0.14f, 0.15f, 0.16f, 0.78f * dockAlpha), dock.height * 0.30f);
    for (std::size_t index = 0; index < layout.dockIcons.size(); ++index) {
        const std::size_t appIndex = DockApps[index];
        const Rect& source = layout.dockIcons[index];
        const float dockIconScale = 0.82f + 0.18f * dockEntrance;
        const float iconWidth = source.width * dockIconScale;
        const float iconHeight = source.height * dockIconScale;
        const Rect iconRect{source.x + (source.width - iconWidth) * 0.5f, source.y + (source.height - iconHeight) * 0.5f + dock.y - layout.dock.y, iconWidth, iconHeight};
        drawIcon(drawer, Apps[appIndex], appIndex, iconRect, dockAlpha);
    }
}

Rect closeButtonRect(const Rect& surface) {
    const float scale = clamp(std::min(surface.width / 480.0f, surface.height / 760.0f), 0.65f, 1.7f);
    const float margin = std::max(20.0f, std::min(surface.width, surface.height) * 0.055f);
    return {surface.x + surface.width - margin - 72.0f * scale, surface.y + margin, 72.0f * scale, 34.0f * scale};
}

void drawAppSurface(Drawer& drawer, const FontAtlas& font, const Rect& surface, float progress, const App& app, std::size_t index) {
    const float scale = clamp(std::min(surface.width / 480.0f, surface.height / 760.0f), 0.55f, 1.7f);
    const float shortSide = std::min(surface.width, surface.height);
    const float margin = std::max(18.0f, shortSide * 0.055f);
    const float radius = 14.0f + shortSide * 0.035f * progress;
    const float contentAlpha = clamp((progress - 0.48f) / 0.52f, 0.0f, 1.0f);

    drawer.roundedRect({surface.x + 3.0f, surface.y + 7.0f, surface.width, surface.height}, rgba(0.0f, 0.0f, 0.0f, 0.28f * progress), radius + 3.0f);
    drawer.roundedRect(surface, rgba(0.045f, 0.052f, 0.056f), radius);
    if (contentAlpha <= 0.0f) {
        return;
    }

    const glm::vec4 white = rgba(0.94f, 0.95f, 0.92f, contentAlpha);
    const glm::vec4 muted = rgba(0.55f, 0.59f, 0.58f, contentAlpha);
    const glm::vec4 accent = rgba(app.accent, contentAlpha);
    const float titleSize = clamp(shortSide * 0.083f, 30.0f, 68.0f) * scale;
    const Rect closeRect = closeButtonRect(surface);

    font.draw(drawer, "FIELD KIT   /   TOOL", surface.x + margin, surface.y + margin, 11.0f * scale, muted);
    drawer.roundedRect(closeRect, rgba(0.12f, 0.14f, 0.15f, contentAlpha), 13.0f * scale);
    font.draw(drawer, "BACK", closeRect.x + closeRect.width * 0.5f, closeRect.y + 8.0f * scale, 10.0f * scale, white, true);

    font.draw(drawer, app.name, surface.x + margin, surface.y + surface.height * 0.16f, titleSize, white);
    font.draw(drawer, app.description, surface.x + margin + 2.0f * scale, surface.y + surface.height * 0.24f, 12.0f * scale, muted);

    const float panelY = surface.y + surface.height * 0.34f;
    const float panelHeight = surface.height * 0.28f;
    const Rect panel{surface.x + margin, panelY, surface.width - 2.0f * margin, panelHeight};
    drawer.roundedRect(panel, rgba(0.085f, 0.10f, 0.10f, contentAlpha), 18.0f * scale);
    font.draw(drawer, "LIVE STUDY", panel.x + 18.0f * scale, panel.y + 15.0f * scale, 10.0f * scale, muted);

    constexpr int barCount = 28;
    const float chartTop = panel.y + panelHeight * 0.34f;
    const float chartHeight = panelHeight * 0.43f;
    const float barGap = 4.0f * scale;
    const float barWidth = (panel.width - 36.0f * scale - (barCount - 1) * barGap) / barCount;
    for (int bar = 0; bar < barCount; ++bar) {
        const float wave = 0.18f + 0.82f * std::abs(std::sin((bar + 1) * (0.46f + index * 0.013f)) * std::cos(bar * 0.17f));
        const float barHeight = chartHeight * wave;
        const float barX = panel.x + 18.0f * scale + bar * (barWidth + barGap);
        drawer.roundedRect({barX, chartTop + chartHeight - barHeight, std::max(1.0f, barWidth), barHeight}, accent, std::min(barWidth, 5.0f * scale) * 0.5f);
    }

    font.draw(drawer, "SESSION", surface.x + margin, surface.y + surface.height * 0.70f, 10.0f * scale, muted);
    font.draw(drawer, "0" + std::to_string(index + 1), surface.x + margin, surface.y + surface.height * 0.735f, 38.0f * scale, accent);
    font.draw(drawer, "A PLACE TO PICK UP WHERE YOU LEFT OFF.", surface.x + margin, surface.y + surface.height * 0.84f, 10.0f * scale, muted);
    drawer.roundedRect({surface.x + margin, surface.y + surface.height * 0.89f, surface.width - 2.0f * margin, 1.0f * scale}, rgba(0.22f, 0.24f, 0.23f, contentAlpha), 0.0f);
    font.draw(drawer, "FIELD NOTES  /  PRIVATE COLLECTION", surface.x + margin, surface.y + surface.height * 0.92f, 9.0f * scale, muted);
}

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

} // namespace

int main() {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Could not initialize GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(480, 760, "Field Kit | OpenGL 4.3", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Could not create an OpenGL 4.3 Core window\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0 || GLAD_GL_VERSION_4_3 == 0) {
        std::cerr << "OpenGL 4.3 Core is not available on this display\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    try {
        {
            Drawer drawer;
            FontAtlas font;
            int selectedIndex = 0;
            float progress = 0.0f;
            float velocity = 0.0f;
            float target = 0.0f;
            float introTime = 0.0f;
            bool wasMouseDown = false;
            bool wasEscapeDown = false;
            bool wasEnterDown = false;
            bool wasSpaceDown = false;
            double previousTime = glfwGetTime();

            while (glfwWindowShouldClose(window) == GLFW_FALSE) {
                glfwPollEvents();
                const double currentTime = glfwGetTime();
                const float deltaTime = std::min(static_cast<float>(currentTime - previousTime), 0.05f);
                previousTime = currentTime;
                introTime += deltaTime;

                int windowWidth = 0;
                int windowHeight = 0;
                int framebufferWidth = 0;
                int framebufferHeight = 0;
                glfwGetWindowSize(window, &windowWidth, &windowHeight);
                glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
                if (windowWidth <= 0 || windowHeight <= 0 || framebufferWidth <= 0 || framebufferHeight <= 0) {
                    continue;
                }

                const float width = static_cast<float>(framebufferWidth);
                const float height = static_cast<float>(framebufferHeight);
                const float cursorScaleX = width / static_cast<float>(windowWidth);
                const float cursorScaleY = height / static_cast<float>(windowHeight);
                double cursorX = 0.0;
                double cursorY = 0.0;
                glfwGetCursorPos(window, &cursorX, &cursorY);
                cursorX *= cursorScaleX;
                cursorY *= cursorScaleY;

                const Layout layout = makeLayout(width, height);
                const bool mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
                const bool clicked = mouseDown && !wasMouseDown;
                const bool escapeDown = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
                const bool enterDown = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
                const bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

                if (target < 0.5f && progress < 0.02f) {
                    if ((enterDown && !wasEnterDown) || (spaceDown && !wasSpaceDown)) {
                        target = 1.0f;
                    } else if (clicked) {
                        for (std::size_t index = 0; index < layout.cells.size(); ++index) {
                            if (contains(layout.cells[index].bounds, static_cast<float>(cursorX), static_cast<float>(cursorY))) {
                                selectedIndex = static_cast<int>(index);
                                target = 1.0f;
                                break;
                            }
                        }
                        if (target < 0.5f && contains(layout.dock, static_cast<float>(cursorX), static_cast<float>(cursorY))) {
                            const float slotWidth = layout.dock.width / 4.0f;
                            const int slot = std::clamp(static_cast<int>((cursorX - layout.dock.x) / slotWidth), 0, 3);
                            selectedIndex = static_cast<int>(DockApps[static_cast<std::size_t>(slot)]);
                            target = 1.0f;
                        }
                    }
                } else if (target > 0.5f) {
                    if (escapeDown && !wasEscapeDown) {
                        target = 0.0f;
                    } else if (clicked && contains(closeButtonRect({0.0f, 0.0f, width, height}), static_cast<float>(cursorX), static_cast<float>(cursorY))) {
                        target = 0.0f;
                    }
                }

                wasMouseDown = mouseDown;
                wasEscapeDown = escapeDown;
                wasEnterDown = enterDown;
                wasSpaceDown = spaceDown;

                const float acceleration = (target - progress) * 230.0f - velocity * 29.0f;
                velocity += acceleration * deltaTime;
                progress = clamp(progress + velocity * deltaTime, 0.0f, 1.04f);
                if (std::abs(target - progress) < 0.001f && std::abs(velocity) < 0.02f) {
                    progress = target;
                    velocity = 0.0f;
                }

                glViewport(0, 0, framebufferWidth, framebufferHeight);
                drawer.setProjection(width, height);
                drawHome(drawer, font, layout, width, height, selectedIndex, progress, introTime);

                if (progress > 0.001f) {
                    drawer.roundedRect({0.0f, 0.0f, width, height}, rgba(0.0f, 0.0f, 0.0f, 0.78f * clamp(progress, 0.0f, 1.0f)), 0.0f);
                    const Rect source = layout.cells[static_cast<std::size_t>(selectedIndex)].icon;
                    const float amount = clamp(progress, 0.0f, 1.0f);
                    const Rect surface{
                        source.x * (1.0f - amount),
                        source.y * (1.0f - amount),
                        source.width + (width - source.width) * amount,
                        source.height + (height - source.height) * amount,
                    };
                    drawAppSurface(drawer, font, surface, amount, Apps[static_cast<std::size_t>(selectedIndex)], static_cast<std::size_t>(selectedIndex));
                }

                glfwSwapBuffers(window);
            }
        }
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}