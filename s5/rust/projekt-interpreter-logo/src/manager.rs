use rand::{random, seq::SliceRandom};

use crate::{
    drawable::Drawable, interpreter::Command, interpreter::Expr, interpreter::Value, turtle::Turtle,
};

#[derive(Debug)]
pub struct Manager {
    color: String,
    repcount: usize,
    turtles: Vec<Turtle>,
    current_turtle: usize,
    writing: bool,
    label_height: f64,
    objects: Vec<Drawable>,
}

impl Manager {
    pub fn new() -> Self {
        Manager {
            color: "#000000".to_string(),
            repcount: 0,
            turtles: vec![Turtle::new()],
            current_turtle: 0,
            writing: true,
            label_height: 10f64,
            objects: vec![],
        }
    }

    fn get_turtle(&mut self) -> &mut Turtle {
        self.turtles.get_mut(self.current_turtle).unwrap()
    }

    pub fn send_command(&mut self, command: &Command) -> Value {
        let mut changed = false;
        let prev_x = self.get_turtle().x;
        let prev_y = self.get_turtle().y;
        match command {
            Command::Forward(distance) => {
                self.get_turtle().forward(*distance);
                changed = self.writing;
            }
            Command::Backward(distance) => {
                self.get_turtle().back(*distance);
                changed = self.writing;
            }
            Command::Left(angle) => {
                self.get_turtle().left(*angle);
            }
            Command::Right(angle) => {
                self.get_turtle().right(*angle);
            }
            Command::PenUp => {
                self.writing = false;
            }
            Command::PenDown => {
                self.writing = true;
            }
            Command::ClearScreen => {
                self.objects.clear();
            }
            Command::Repeat(reps, body) => {
                self.repcount = 0;
                let mut result = vec![];
                (0..*reps as usize).for_each(|_| {
                    let mut extra = body.clone();
                    extra.push(Expr::Command(Command::NextIter));
                    result.extend(extra);
                });
                return Value::List(result);
            }
            Command::RepCount => return Value::Number(self.repcount as f64),
            Command::NextIter => {
                self.repcount += 1;
            }
            Command::SetColor(color) => {
                self.color = color.to_string();
            }
            Command::Pick(choices) => {
                return Value::Expr(choices.choose(&mut rand::thread_rng()).cloned().unwrap())
            }
            Command::Label(text) => {
                let x = self.get_turtle().x;
                let y = self.get_turtle().y;
                let label_height = self.label_height;
                let angle = self.get_turtle().angle;
                let color = self.color.to_string();
                self.objects.push(Drawable::Text(
                    text.to_string(),
                    x,
                    y,
                    label_height,
                    angle,
                    color,
                ));
            }
            Command::SetLabelHeight(height) => {
                self.label_height = *height;
            }
            Command::Window => (),
            Command::Wrap => (),
            Command::SetTurtle(index) => {
                self.current_turtle = *index - 1;
                while self.turtles.len() < *index {
                    self.turtles.push(Turtle::new());
                }
            }
            Command::Wait(_) => (),
            Command::Random(range) => return Value::Number(range * random::<f64>()),
            Command::ShowTurtle => (),
            Command::HideTurtle => (),
        };

        if changed {
            let new_x = self.get_turtle().x;
            let new_y = self.get_turtle().y;
            let color = self.color.to_string();
            self.objects
                .push(Drawable::Line(prev_x, prev_y, new_x, new_y, color));
        }

        Value::Command(command.clone())
    }

    pub fn save(&self, filename: String) {
        let mut min_x = std::f64::MAX;
        let mut min_y = std::f64::MAX;
        let mut max_x = std::f64::MIN;
        let mut max_y = std::f64::MIN;

        let mut tags = vec![];

        for object in self.objects.iter() {
            match object {
                Drawable::Line(x1, y1, x2, y2, color) => {
                    min_x = [min_x, *x1, *x2].into_iter().reduce(f64::min).unwrap();
                    min_y = [min_y, *y1, *y2].into_iter().reduce(f64::min).unwrap();
                    max_x = [max_x, *x1, *x2].into_iter().reduce(f64::max).unwrap();
                    max_y = [max_y, *y1, *y2].into_iter().reduce(f64::max).unwrap();

                    tags.push(format!(r#"<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{color}" stroke-width="1" />"#));
                }
                Drawable::Text(content, x, y, size, angle, color) => {
                    min_x = min_x.min(*x);
                    min_y = min_y.min(*y);
                    max_x = max_x.max(*x);
                    max_y = max_y.max(*y);

                    let angle = angle + 90f64;
                    // tags.push(format!("<text font-size=\"{size}\" stroke=\"{color}\" stroke-width=\"1\"><tspan x=\"{x}\" y=\"{y}\" rotate=\"{angle}\">{content}</tspan></text>\n"));
                    tags.push(format!(r#"<text text-anchor="start" font-size="{size}" fill="{color}" stroke-width="1" transform="translate({x}, {y}) rotate({angle})">{content}</text>"#));
                }
            }
        }

        const MARGIN: f64 = 10f64;
        let s = format!(
            r#"<svg viewBox="{} {} {} {}" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
{}
</svg>"#,
            min_x - MARGIN,
            min_y - MARGIN,
            max_x - min_x + MARGIN * 2f64,
            max_y - min_y + MARGIN * 2f64,
            tags.join("\n")
        );

        let _ = std::fs::write(filename, s);
    }
}
