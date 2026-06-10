#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 80
#define HEIGHT 25
#define MAX_SHAPES 100

// Define shape types
typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Define shape structure
typedef struct {
    int id;
    ShapeType type;
    union {
        struct { int x1, y1, x2, y2; } line;
        struct { int x, y, w, h; } rect;
        struct { int cx, cy, r; } circle;
        struct { int x1, y1, x2, y2, x3, y3; } triangle;
    } data;
} Shape;

// Global state
Shape shapes[MAX_SHAPES];
int shape_count = 0;
int next_id = 1;
char canvas[HEIGHT][WIDTH];

// Initialize canvas with background characters
void init_canvas(char canvas[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            canvas[y][x] = '_';
        }
    }
}

// Display canvas to console with a custom border
void display_canvas(char canvas[HEIGHT][WIDTH]) {
    // Print top border
    printf("+");
    for (int x = 0; x < WIDTH; x++) {
        printf("-");
    }
    printf("+\n");

    // Print rows
    for (int y = 0; y < HEIGHT; y++) {
        printf("|");
        for (int x = 0; x < WIDTH; x++) {
            putchar(canvas[y][x]);
        }
        printf("|\n");
    }

    // Print bottom border
    printf("+");
    for (int x = 0; x < WIDTH; x++) {
        printf("-");
    }
    printf("+\n");
}

// --- DRAWING ALGORITHMS ---

// Bresenham's Line Algorithm
void draw_line(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        // Draw pixel if it is within canvas boundaries
        if (x1 >= 0 && x1 < WIDTH && y1 >= 0 && y1 < HEIGHT) {
            canvas[y1][x1] = '*';
        }
        
        if (x1 == x2 && y1 == y2) {
            break;
        }
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Rectangle drawing using 4 lines
void draw_rectangle(char canvas[HEIGHT][WIDTH], int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    
    // Draw top border
    draw_line(canvas, x, y, x + w - 1, y);
    // Draw bottom border
    draw_line(canvas, x, y + h - 1, x + w - 1, y + h - 1);
    // Draw left border
    draw_line(canvas, x, y, x, y + h - 1);
    // Draw right border
    draw_line(canvas, x + w - 1, y, x + w - 1, y + h - 1);
}

// Helper function to plot symmetric points of a circle
void draw_circle_points(char canvas[HEIGHT][WIDTH], int cx, int cy, int x, int y) {
    int px[8] = {cx + x, cx - x, cx + x, cx - x, cx + y, cx - y, cx + y, cx - y};
    int py[8] = {cy + y, cy + y, cy - y, cy - y, cy + x, cy + x, cy - x, cy - x};
    
    for (int i = 0; i < 8; i++) {
        if (px[i] >= 0 && px[i] < WIDTH && py[i] >= 0 && py[i] < HEIGHT) {
            canvas[py[i]][px[i]] = '*';
        }
    }
}

// Bresenham's / Midpoint Circle drawing algorithm
void draw_circle(char canvas[HEIGHT][WIDTH], int cx, int cy, int r) {
    if (r < 0) return;
    if (r == 0) {
        if (cx >= 0 && cx < WIDTH && cy >= 0 && cy < HEIGHT) {
            canvas[cy][cx] = '*';
        }
        return;
    }
    
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    
    draw_circle_points(canvas, cx, cy, x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        draw_circle_points(canvas, cx, cy, x, y);
    }
}

// Triangle drawing using 3 lines
void draw_triangle(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(canvas, x1, y1, x2, y2);
    draw_line(canvas, x2, y2, x3, y3);
    draw_line(canvas, x3, y3, x1, y1);
}

// Render all shapes to the canvas
void render_all(char canvas[HEIGHT][WIDTH], Shape shapes[], int count) {
    init_canvas(canvas);
    for (int i = 0; i < count; i++) {
        switch (shapes[i].type) {
            case SHAPE_LINE:
                draw_line(canvas, shapes[i].data.line.x1, shapes[i].data.line.y1,
                                  shapes[i].data.line.x2, shapes[i].data.line.y2);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(canvas, shapes[i].data.rect.x, shapes[i].data.rect.y,
                                       shapes[i].data.rect.w, shapes[i].data.rect.h);
                break;
            case SHAPE_CIRCLE:
                draw_circle(canvas, shapes[i].data.circle.cx, shapes[i].data.circle.cy,
                                    shapes[i].data.circle.r);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(canvas, shapes[i].data.triangle.x1, shapes[i].data.triangle.y1,
                                      shapes[i].data.triangle.x2, shapes[i].data.triangle.y2,
                                      shapes[i].data.triangle.x3, shapes[i].data.triangle.y3);
                break;
        }
    }
}

// --- USER INPUT UTILITIES ---

// Safe integer input with range checks
int get_int_input(const char* prompt, int min_val, int max_val) {
    char line[100];
    int val;
    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin)) {
            if (sscanf(line, "%d", &val) == 1) {
                if (val >= min_val && val <= max_val) {
                    return val;
                } else {
                    printf("Error: Input must be between %d and %d.\n", min_val, max_val);
                }
            } else {
                printf("Error: Invalid input format. Please enter an integer.\n");
            }
        }
    }
}

// Safe default integer input for modifying shapes
int get_int_input_default(const char* prompt, int min_val, int max_val, int default_val) {
    char line[100];
    int val;
    while (1) {
        printf("%s (current: %d, press Enter to keep): ", prompt, default_val);
        if (fgets(line, sizeof(line), stdin)) {
            // Check if user just pressed Enter (empty line)
            if (line[0] == '\n' || line[0] == '\r') {
                return default_val;
            }
            if (sscanf(line, "%d", &val) == 1) {
                if (val >= min_val && val <= max_val) {
                    return val;
                } else {
                    printf("Error: Input must be between %d and %d.\n", min_val, max_val);
                }
            } else {
                printf("Error: Invalid input format. Please enter an integer.\n");
            }
        }
    }
}

// --- SHAPE MANAGEMENT MENUS ---

void print_shape_info(Shape s) {
    switch (s.type) {
        case SHAPE_LINE:
            printf("ID: %d | Line | Start: (%d, %d), End: (%d, %d)\n", 
                   s.id, s.data.line.x1, s.data.line.y1, s.data.line.x2, s.data.line.y2);
            break;
        case SHAPE_RECTANGLE:
            printf("ID: %d | Rectangle | Top-Left: (%d, %d), Size: %dx%d\n", 
                   s.id, s.data.rect.x, s.data.rect.y, s.data.rect.w, s.data.rect.h);
            break;
        case SHAPE_CIRCLE:
            printf("ID: %d | Circle | Center: (%d, %d), Radius: %d\n", 
                   s.id, s.data.circle.cx, s.data.circle.cy, s.data.circle.r);
            break;
        case SHAPE_TRIANGLE:
            printf("ID: %d | Triangle | V1: (%d, %d), V2: (%d, %d), V3: (%d, %d)\n", 
                   s.id, s.data.triangle.x1, s.data.triangle.y1, 
                   s.data.triangle.x2, s.data.triangle.y2, 
                   s.data.triangle.x3, s.data.triangle.y3);
            break;
    }
}

void list_shapes() {
    if (shape_count == 0) {
        printf("No active shapes on the canvas.\n");
        return;
    }
    printf("\n--- Active Shapes ---\n");
    for (int i = 0; i < shape_count; i++) {
        print_shape_info(shapes[i]);
    }
}

void add_shape_menu() {
    if (shape_count >= MAX_SHAPES) {
        printf("Error: Maximum shape limit reached (%d).\n", MAX_SHAPES);
        return;
    }
    
    printf("\n--- Add a Shape ---\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    int choice = get_int_input("Choose shape type: ", 1, 4);

    Shape s;
    s.id = next_id++;

    switch (choice) {
        case 1:
            s.type = SHAPE_LINE;
            printf("\nEnter Line properties:\n");
            s.data.line.x1 = get_int_input("Start X (0-79): ", 0, WIDTH - 1);
            s.data.line.y1 = get_int_input("Start Y (0-24): ", 0, HEIGHT - 1);
            s.data.line.x2 = get_int_input("End X (0-79): ", 0, WIDTH - 1);
            s.data.line.y2 = get_int_input("End Y (0-24): ", 0, HEIGHT - 1);
            break;
        case 2:
            s.type = SHAPE_RECTANGLE;
            printf("\nEnter Rectangle properties:\n");
            s.data.rect.x = get_int_input("Top-Left X (0-79): ", 0, WIDTH - 1);
            s.data.rect.y = get_int_input("Top-Left Y (0-24): ", 0, HEIGHT - 1);
            s.data.rect.w = get_int_input("Width (1-80): ", 1, WIDTH);
            s.data.rect.h = get_int_input("Height (1-25): ", 1, HEIGHT);
            break;
        case 3:
            s.type = SHAPE_CIRCLE;
            printf("\nEnter Circle properties:\n");
            s.data.circle.cx = get_int_input("Center X (0-79): ", 0, WIDTH - 1);
            s.data.circle.cy = get_int_input("Center Y (0-24): ", 0, HEIGHT - 1);
            s.data.circle.r = get_int_input("Radius (0-80): ", 0, 80);
            break;
        case 4:
            s.type = SHAPE_TRIANGLE;
            printf("\nEnter Triangle properties:\n");
            s.data.triangle.x1 = get_int_input("Vertex 1 X (0-79): ", 0, WIDTH - 1);
            s.data.triangle.y1 = get_int_input("Vertex 1 Y (0-24): ", 0, HEIGHT - 1);
            s.data.triangle.x2 = get_int_input("Vertex 2 X (0-79): ", 0, WIDTH - 1);
            s.data.triangle.y2 = get_int_input("Vertex 2 Y (0-24): ", 0, HEIGHT - 1);
            s.data.triangle.x3 = get_int_input("Vertex 3 X (0-79): ", 0, WIDTH - 1);
            s.data.triangle.y3 = get_int_input("Vertex 3 Y (0-24): ", 0, HEIGHT - 1);
            break;
    }

    shapes[shape_count++] = s;
    printf("\nShape added successfully with ID %d!\n", s.id);
}

void delete_shape_menu() {
    if (shape_count == 0) {
        printf("No active shapes to delete.\n");
        return;
    }
    
    list_shapes();
    int target_id = get_int_input("\nEnter the ID of the shape to delete: ", 1, next_id - 1);
    int found_index = -1;
    
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == target_id) {
            found_index = i;
            break;
        }
    }

    if (found_index == -1) {
        printf("Error: Shape with ID %d not found.\n", target_id);
        return;
    }

    // Shift shapes array to remove the deleted shape
    for (int i = found_index; i < shape_count - 1; i++) {
        shapes[i] = shapes[i + 1];
    }
    shape_count--;
    printf("Shape %d deleted successfully!\n", target_id);
}

void modify_shape_menu() {
    if (shape_count == 0) {
        printf("No active shapes to modify.\n");
        return;
    }
    
    list_shapes();
    int target_id = get_int_input("\nEnter the ID of the shape to modify: ", 1, next_id - 1);
    int found_index = -1;
    
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == target_id) {
            found_index = i;
            break;
        }
    }

    if (found_index == -1) {
        printf("Error: Shape with ID %d not found.\n", target_id);
        return;
    }

    Shape *s = &shapes[found_index];
    printf("\nModifying Shape %d:\n", s->id);
    print_shape_info(*s);

    switch (s->type) {
        case SHAPE_LINE:
            s->data.line.x1 = get_int_input_default("Start X", 0, WIDTH - 1, s->data.line.x1);
            s->data.line.y1 = get_int_input_default("Start Y", 0, HEIGHT - 1, s->data.line.y1);
            s->data.line.x2 = get_int_input_default("End X", 0, WIDTH - 1, s->data.line.x2);
            s->data.line.y2 = get_int_input_default("End Y", 0, HEIGHT - 1, s->data.line.y2);
            break;
        case SHAPE_RECTANGLE:
            s->data.rect.x = get_int_input_default("Top-Left X", 0, WIDTH - 1, s->data.rect.x);
            s->data.rect.y = get_int_input_default("Top-Left Y", 0, HEIGHT - 1, s->data.rect.y);
            s->data.rect.w = get_int_input_default("Width", 1, WIDTH, s->data.rect.w);
            s->data.rect.h = get_int_input_default("Height", 1, HEIGHT, s->data.rect.h);
            break;
        case SHAPE_CIRCLE:
            s->data.circle.cx = get_int_input_default("Center X", 0, WIDTH - 1, s->data.circle.cx);
            s->data.circle.cy = get_int_input_default("Center Y", 0, HEIGHT - 1, s->data.circle.cy);
            s->data.circle.r = get_int_input_default("Radius", 0, 80, s->data.circle.r);
            break;
        case SHAPE_TRIANGLE:
            s->data.triangle.x1 = get_int_input_default("Vertex 1 X", 0, WIDTH - 1, s->data.triangle.x1);
            s->data.triangle.y1 = get_int_input_default("Vertex 1 Y", 0, HEIGHT - 1, s->data.triangle.y1);
            s->data.triangle.x2 = get_int_input_default("Vertex 2 X", 0, WIDTH - 1, s->data.triangle.x2);
            s->data.triangle.y2 = get_int_input_default("Vertex 2 Y", 0, HEIGHT - 1, s->data.triangle.y2);
            s->data.triangle.x3 = get_int_input_default("Vertex 3 X", 0, WIDTH - 1, s->data.triangle.x3);
            s->data.triangle.y3 = get_int_input_default("Vertex 3 Y", 0, HEIGHT - 1, s->data.triangle.y3);
            break;
    }

    printf("Shape %d modified successfully!\n", s->id);
}

void reset_canvas_menu() {
    shape_count = 0;
    next_id = 1;
    printf("Canvas reset! All shapes removed.\n");
}

int main() {
    // Initial render
    render_all(canvas, shapes, shape_count);

    while (1) {
        printf("\n");
        display_canvas(canvas);
        printf("\n===================================\n");
        printf("      2D Character Graphics Editor\n");
        printf("===================================\n");
        printf("Canvas size: %d x %d (X: 0-%d, Y: 0-%d)\n", WIDTH, HEIGHT, WIDTH - 1, HEIGHT - 1);
        printf("Foreground: '*'  Background: '_'\n");
        printf("-----------------------------------\n");
        printf("1. Add a shape\n");
        printf("2. Delete a shape\n");
        printf("3. Modify a shape\n");
        printf("4. Display canvas\n");
        printf("5. List active shapes\n");
        printf("6. Reset canvas\n");
        printf("7. Exit\n");
        printf("===================================\n");

        int choice = get_int_input("Enter choice (1-7): ", 1, 7);
        if (choice == 7) {
            printf("\nExiting 2D Graphics Editor. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1:
                add_shape_menu();
                break;
            case 2:
                delete_shape_menu();
                break;
            case 3:
                modify_shape_menu();
                break;
            case 4:
                // Canvas is already displayed at the top of the loop,
                // but we print a clear notification that we are updating it.
                printf("\nRedrawing canvas...\n");
                break;
            case 5:
                list_shapes();
                break;
            case 6:
                reset_canvas_menu();
                break;
        }
        
        // Re-render state to canvas for the next iteration
        render_all(canvas, shapes, shape_count);
    }
    
    return 0;
}
