import raylib as rl

def main():
    screen_width = 400
    screen_height = 700
    rl.InitWindow(screen_width, screen_height, b"iOS Physics Simulator")
    rl.SetTargetFPS(60)

    current_y = 700.0 
    target_y = 700.0  
    velocity_y = 0.0    

    stiffness = 180.0   
    damping = 18.0      

    while not rl.WindowShouldClose():
        dt = rl.GetFrameTime()
        if dt > 0.1: dt = 0.1

        if rl.IsKeyPressed(rl.KEY_SPACE) or rl.IsMouseButtonPressed(rl.MOUSE_BUTTON_LEFT):
            if target_y > 400.0:
                target_y = 250.0 
            else:
                target_y = 700.0

        displacement = current_y - target_y
        spring_force = -stiffness * displacement
        damping_force = -damping * velocity_y
        
        acceleration = spring_force + damping_force
        velocity_y += acceleration * dt
        current_y += velocity_y * dt

        rl.BeginDrawing()
        rl.ClearBackground(rl.Color(20, 20, 25, 255))

        rl.DrawText(b"iOS 60Hz Physics", 40, 80, 24, rl.Color(150, 150, 160, 255))
        rl.DrawText(b"Click or Press SPACE to trigger", 40, 120, 16, rl.Color(90, 90, 100, 255))

        card_y_int = int(current_y)

        rl.DrawRectangleRounded(
            rl.Rectangle(20, float(card_y_int), 360, 420), 
            0.08, 16, rl.Color(35, 35, 45, 255)
        )

        rl.DrawRectangleRounded(
            rl.Rectangle(170, float(card_y_int + 15), 60, 6), 
            0.5, 4, rl.Color(80, 80, 90, 255)
        )

        rl.DrawText(b"Control Center", 45, card_y_int + 45, 20, rl.WHITE)
        rl.DrawRectangle(45, card_y_int + 90, 140, 100, rl.Color(50, 50, 65, 255))
        rl.DrawRectangle(215, card_y_int + 90, 140, 100, rl.Color(50, 50, 65, 255))

        rl.EndDrawing()

    rl.CloseWindow()

if __name__ == "__main__":
    main()
