#[derive(Debug)]
pub enum Drawable {
    // x1, y1, x2, y2, color
    Line(f64, f64, f64, f64, String),
    // content, x, y, size, angle, color
    Text(String, f64, f64, f64, f64, String),
}
