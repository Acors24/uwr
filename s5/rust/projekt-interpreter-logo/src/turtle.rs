#[derive(Debug)]
pub struct Turtle {
    pub x: f64,
    pub y: f64,
    pub angle: f64,
}

impl Turtle {
    pub fn new() -> Self {
        Turtle {
            x: 0f64,
            y: 0f64,
            angle: 0f64,
        }
    }

    pub fn forward(&mut self, distance: f64) {
        let angle = self.angle.to_radians();
        self.x += angle.sin() * distance;
        self.y -= angle.cos() * distance;
    }

    pub fn back(&mut self, distance: f64) {
        let angle = self.angle.to_radians();
        self.x -= angle.sin() * distance;
        self.y += angle.cos() * distance;
    }

    pub fn left(&mut self, angle: f64) {
        self.angle -= angle;
    }

    pub fn right(&mut self, angle: f64) {
        self.angle += angle;
    }
}
