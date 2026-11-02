import math

import raylib as rl


INITIAL_WINDOW_WIDTH = 400
INITIAL_WINDOW_HEIGHT = 700
WORKSPACES = (
    {"name": b"Tempo", "tag": b"SOUND / 04", "accent": (218, 237, 111), "sub": b"A rhythm in progress", "metric": b"02:48", "unit": b"MINUTES IN FLOW"},
    {"name": b"Canvas", "tag": b"SHAPES / 12", "accent": (255, 135, 105), "sub": b"Color study, no. 12", "metric": b"08", "unit": b"LAYERS ACTIVE"},
    {"name": b"Notes", "tag": b"THOUGHTS / 08", "accent": (124, 201, 188), "sub": b"Collected along the way", "metric": b"14", "unit": b"LINES CAPTURED"},
    {"name": b"Index", "tag": b"COLLECTION / 31", "accent": (225, 190, 130), "sub": b"A growing set of finds", "metric": b"31", "unit": b"ITEMS SAVED"},
)


def color(r, g, b, a=255):
    return rl.ffi.new("Color *", (int(r), int(g), int(b), int(a)))[0]


def rectangle(x, y, width, height):
    return rl.ffi.new("Rectangle *", (float(x), float(y), float(width), float(height)))[0]


def clamp(value, low, high):
    return max(low, min(high, value))


def draw_text(text, x, y, font_size, fill, scale=1.0):
    rl.DrawText(text, round(x), round(y), max(1, round(font_size * scale)), fill)


def draw_round_rect(x, y, width, height, radius, fill):
    if width <= 0 or height <= 0:
        return
    roundness = min(1.0, (radius * 2.0) / min(width, height))
    rl.DrawRectangleRounded(rectangle(x, y, width, height), roundness, 16, fill)


def draw_tile_icon(index, x, y, accent, scale):
    unit = 44 * scale
    ink = color(21, 43, 39)
    draw_round_rect(x, y, unit, unit, 13 * scale, accent)

    if index == 0:
        heights = (12, 21, 30, 18, 25)
        for bar, height in enumerate(heights):
            draw_round_rect(x + (9 + bar * 6) * scale, y + (44 - height) * scale / 2, 3 * scale, height * scale, 2 * scale, ink)
    elif index == 1:
        rl.DrawLine(round(x + 10 * scale), round(y + 31 * scale), round(x + 20 * scale), round(y + 12 * scale), ink)
        rl.DrawLine(round(x + 20 * scale), round(y + 12 * scale), round(x + 28 * scale), round(y + 28 * scale), ink)
        rl.DrawLine(round(x + 28 * scale), round(y + 28 * scale), round(x + 35 * scale), round(y + 16 * scale), ink)
        rl.DrawCircle(round(x + 20 * scale), round(y + 12 * scale), 3 * scale, ink)
    elif index == 2:
        for row, width in enumerate((24, 18, 22)):
            draw_round_rect(x + 10 * scale, y + (12 + row * 8) * scale, width * scale, 3 * scale, scale, ink)
        rl.DrawCircle(round(x + 34 * scale), round(y + 33 * scale), 3 * scale, ink)
    else:
        draw_round_rect(x + 10 * scale, y + 10 * scale, 24 * scale, 24 * scale, 5 * scale, ink)
        draw_round_rect(x + 14 * scale, y + 14 * scale, 16 * scale, 16 * scale, 3 * scale, accent)
        rl.DrawLine(round(x + 17 * scale), round(y + 19 * scale), round(x + 27 * scale), round(y + 19 * scale), ink)
        rl.DrawLine(round(x + 17 * scale), round(y + 24 * scale), round(x + 25 * scale), round(y + 24 * scale), ink)


def home_layout(width, height):
    scale = clamp(min(width / 400.0, height / 700.0), 0.8, 1.75)
    margin = max(18.0, width * 0.06)
    gap = max(12.0, width * 0.05)
    tile_width = (width - 2 * margin - gap) / 2
    tile_height = min(height * 0.185, width * 0.38)
    first_y = height * 0.35
    row_gap = height * 0.035
    tiles = (
        (margin, first_y, tile_width, tile_height),
        (margin + tile_width + gap, first_y, tile_width, tile_height),
        (margin, first_y + tile_height + row_gap, tile_width, tile_height),
        (margin + tile_width + gap, first_y + tile_height + row_gap, tile_width, tile_height),
    )
    return scale, margin, tiles


def draw_home(width, height, selected_index, scale, margin, tiles):
    rl.ClearBackground(color(17, 35, 32))

    grid_step = max(24, round(36 * scale))
    for y in range(round(18 * scale), height, grid_step):
        rl.DrawLine(0, y, width, y, color(25, 46, 41, 110))

    draw_text(b"FIELD / 01", margin, height * 0.04, 13, color(218, 237, 111), scale)
    draw_text(b"STUDIO", width - margin - 48 * scale, height * 0.04, 12, color(139, 164, 151), scale)
    rl.DrawLine(round(margin), round(height * 0.08), round(width - margin), round(height * 0.08), color(57, 78, 68))
    draw_text(b"Find your", margin, height * 0.115, 34, color(239, 239, 218), scale)
    draw_text(b"next rhythm.", margin, height * 0.17, 34, color(239, 239, 218), scale)
    draw_text(b"WORKSPACES", margin, height * 0.27, 11, color(143, 166, 152), scale)
    draw_text(b"04", width - margin - 20 * scale, height * 0.27, 11, color(218, 237, 111), scale)

    for index, workspace in enumerate(WORKSPACES):
        x, y, tile_width, tile_height = tiles[index]
        selected = index == selected_index
        panel = color(37, 61, 54) if selected else color(29, 51, 46)
        draw_round_rect(x, y, tile_width, tile_height, 12 * scale, panel)
        draw_tile_icon(index, x + 14 * scale, y + 12 * scale, color(*workspace["accent"]), scale)
        draw_text(workspace["tag"], x + 14 * scale, y + tile_height - 49 * scale, 10, color(145, 169, 153), scale)
        draw_text(workspace["name"], x + 14 * scale, y + tile_height - 29 * scale, 20, color(239, 239, 218), scale)
        rl.DrawCircle(round(x + tile_width - 18 * scale), round(y + tile_height - 19 * scale), 3 * scale, color(*workspace["accent"]))

    footer_y = height * 0.79
    rl.DrawLine(round(margin), round(footer_y), round(width - margin), round(footer_y), color(57, 78, 68))
    draw_text(b"MONDAY", margin, footer_y + 22 * scale, 11, color(143, 166, 152), scale)
    draw_text(b"A GOOD DAY TO MAKE", margin, footer_y + 48 * scale, 14, color(218, 237, 111), scale)
    draw_text(b"FIELD NOTES  /  VOL. 01", margin, height - 32 * scale, 10, color(109, 136, 122), scale)


def draw_workspace(rect, progress, workspace, workspace_index):
    x, y, width, height = rect
    scale = clamp(min(width / 400.0, height / 700.0), 0.72, 1.75)
    margin = max(20.0, min(width, height) * 0.06)
    radius = 12 + min(width, height) * 0.025 * progress
    alpha = int(255 * clamp((progress - 0.58) / 0.42, 0.0, 1.0))

    draw_round_rect(x + 3, y + 7, width, height, radius + 3, color(0, 0, 0, int(70 * progress)))
    draw_round_rect(x, y, width, height, radius, color(233, 235, 214))

    if alpha == 0:
        return

    ink = color(24, 47, 42, alpha)
    muted = color(94, 119, 105, alpha)
    accent = color(*workspace["accent"], alpha)

    close_x = x + width - margin - 20 * scale
    close_y = y + margin + 20 * scale
    draw_text(b"FIELD / WORKSPACE", x + margin, y + margin, 11, muted, scale)
    rl.DrawCircle(round(close_x), round(close_y), 18 * scale, color(219, 224, 199, alpha))
    rl.DrawLine(round(close_x - 5 * scale), round(close_y - 5 * scale), round(close_x + 5 * scale), round(close_y + 5 * scale), ink)
    rl.DrawLine(round(close_x + 5 * scale), round(close_y - 5 * scale), round(close_x - 5 * scale), round(close_y + 5 * scale), ink)

    draw_text(workspace["name"], x + margin, y + height * 0.13, 36, ink, scale)
    draw_text(workspace["sub"], x + margin, y + height * 0.20, 14, muted, scale)

    chart_x = x + margin
    chart_y = y + height * 0.29
    chart_width = width - 2 * margin
    chart_height = height * 0.29
    draw_round_rect(chart_x, chart_y, chart_width, chart_height, 15, color(27, 55, 48, alpha))
    draw_text(b"LIVE STUDY", chart_x + 16 * scale, chart_y + 14 * scale, 10, color(170, 190, 163, alpha), scale)
    draw_text(b"01", chart_x + chart_width - 31 * scale, chart_y + 14 * scale, 10, color(170, 190, 163, alpha), scale)

    wave_color = color(*workspace["accent"], alpha)
    previous_x = round(chart_x + 16 * scale)
    previous_y = round(chart_y + chart_height * 0.57)
    for step in range(1, 49):
        point_x = round(chart_x + 16 * scale + (chart_width - 32 * scale) * step / 48)
        phase = step / 48 * math.tau * (1.4 + workspace_index * 0.18)
        envelope = math.sin(math.pi * step / 48) * chart_height * 0.19
        point_y = round(chart_y + chart_height * 0.57 - math.sin(phase) * envelope)
        rl.DrawLine(previous_x, previous_y, point_x, point_y, wave_color)
        previous_x = point_x
        previous_y = point_y

    baseline_y = chart_y + chart_height * 0.78
    rl.DrawLine(round(chart_x + 16 * scale), round(baseline_y), round(chart_x + chart_width - 16 * scale), round(baseline_y), color(68, 91, 77, alpha))
    draw_text(b"START", chart_x + 16 * scale, chart_y + chart_height - 23 * scale, 9, color(170, 190, 163, alpha), scale)
    draw_text(b"NOW", chart_x + chart_width - 42 * scale, chart_y + chart_height - 23 * scale, 9, color(170, 190, 163, alpha), scale)

    draw_text(workspace["metric"], x + margin, y + height * 0.66, 39, ink, scale)
    draw_text(workspace["unit"], x + margin + 2 * scale, y + height * 0.74, 10, muted, scale)
    footer_line_y = y + height * 0.86
    rl.DrawLine(round(x + margin), round(footer_line_y), round(x + width - margin), round(footer_line_y), color(183, 197, 174, alpha))
    draw_text(b"FIELD JOURNAL", x + margin, y + height * 0.90, 10, muted, scale)
    draw_text(b"SESSION  /  001", x + width - margin - 116 * scale, y + height * 0.90, 10, muted, scale)


def point_in_rect(point_x, point_y, rect):
    x, y, width, height = rect
    return x <= point_x <= x + width and y <= point_y <= y + height


def main():
    rl.SetConfigFlags(rl.FLAG_WINDOW_RESIZABLE)
    rl.InitWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, b"Field Studio - Motion Study")
    rl.SetWindowMinSize(320, 480)
    rl.SetTargetFPS(60)

    progress = 0.0
    progress_velocity = 0.0
    target_progress = 0.0
    selected_index = 0

    while not rl.WindowShouldClose():
        dt = min(rl.GetFrameTime(), 0.05)
        window_width = rl.GetScreenWidth()
        window_height = rl.GetScreenHeight()
        scale, margin, tiles = home_layout(window_width, window_height)
        mouse_pressed = rl.IsMouseButtonPressed(rl.MOUSE_BUTTON_LEFT)
        mouse_x = rl.GetMouseX()
        mouse_y = rl.GetMouseY()

        if target_progress == 0.0 and progress < 0.02:
            if rl.IsKeyPressed(rl.KEY_SPACE) or rl.IsKeyPressed(rl.KEY_ENTER):
                target_progress = 1.0
            elif mouse_pressed:
                for index, tile in enumerate(tiles):
                    if point_in_rect(mouse_x, mouse_y, tile):
                        selected_index = index
                        target_progress = 1.0
                        break
        elif target_progress == 1.0:
            if rl.IsKeyPressed(rl.KEY_ESCAPE):
                target_progress = 0.0
            elif mouse_pressed:
                close_rect = (window_width - margin - 40 * scale, margin, 40 * scale, 40 * scale)
                if point_in_rect(mouse_x, mouse_y, close_rect):
                    target_progress = 0.0

        acceleration = (target_progress - progress) * 220.0 - progress_velocity * 27.0
        progress_velocity += acceleration * dt
        progress += progress_velocity * dt
        progress = clamp(progress, 0.0, 1.04)
        if abs(target_progress - progress) < 0.001 and abs(progress_velocity) < 0.02:
            progress = target_progress
            progress_velocity = 0.0

        rl.BeginDrawing()
        draw_home(window_width, window_height, selected_index, scale, margin, tiles)

        if progress > 0.001:
            backdrop_alpha = int(165 * clamp(progress, 0.0, 1.0))
            rl.DrawRectangle(0, 0, window_width, window_height, color(7, 17, 15, backdrop_alpha))

            source_x, source_y, source_width, source_height = tiles[selected_index]
            destination = (0.0, 0.0, float(window_width), float(window_height))
            amount = clamp(progress, 0.0, 1.0)
            app_rect = (
                source_x + (destination[0] - source_x) * amount,
                source_y + (destination[1] - source_y) * amount,
                source_width + (destination[2] - source_width) * amount,
                source_height + (destination[3] - source_height) * amount,
            )
            draw_workspace(app_rect, amount, WORKSPACES[selected_index], selected_index)

        rl.EndDrawing()

    rl.CloseWindow()


if __name__ == "__main__":
    main()
