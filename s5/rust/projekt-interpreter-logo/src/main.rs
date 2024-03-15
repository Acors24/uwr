use interpreter::interpret;

mod drawable;
mod interpreter;
mod manager;
mod turtle;

fn main() {
    interpret("logo_src/star.txt", "star").ok();
}

#[cfg(test)]
mod tests {
    use crate::interpreter::interpret;

    #[test]
    fn fern() {
        interpret("logo_src/fern.txt", "fern").ok();
    }

    #[test]
    fn star() {
        interpret("logo_src/star.txt", "star").ok();
    }

    #[test]
    fn tree() {
        interpret("logo_src/tree.txt", "tree").ok();
    }

    #[test]
    fn race() {
        interpret("logo_src/race.txt", "race").ok();
    }

    #[test]
    fn square() {
        interpret("logo_src/square.txt", "square").ok();
    }

    #[test]
    fn spiral() {
        interpret("logo_src/spiral.txt", "spiral").ok();
    }

    #[test]
    fn spiral_2() {
        interpret("logo_src/spiral_2.txt", "spiral_2").ok();
    }

    #[test]
    fn spiral_3() {
        interpret("logo_src/spiral_3.txt", "spiral_3").ok();
    }

    #[test]
    fn circle() {
        interpret("logo_src/circle.txt", "circle").ok();
    }

    #[test]
    fn colors() {
        interpret("logo_src/colors.txt", "colors").ok();
    }

    #[test]
    fn fib() {
        interpret("logo_src/fib.txt", "fib").ok();
    }
}
