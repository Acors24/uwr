use std::error::Error;
use std::{collections::HashMap, iter::Peekable};

use crate::manager::Manager;

pub fn interpret(source_filename: &str, result_filename: &str) -> Result<(), Box<dyn Error>> {
    let source = std::fs::read_to_string(source_filename)?;

    let keywords = HashMap::from([
        ("to"    , Keyword::To),
        ("end"   , Keyword::End),
        ("if"    , Keyword::If),
        ("stop"  , Keyword::Stop),
        ("red"   , Keyword::Red),
        ("orange", Keyword::Orange),
        ("yellow", Keyword::Yellow),
        ("green" , Keyword::Green),
        ("blue"  , Keyword::Blue),
        ("violet", Keyword::Violet),
    ]);

    let tokens = tokenize(&source, &keywords);

    let builtins = vec![
        Builtin::new(
            "fd",
            1,
            Box::new(|args| {
                if let Value::Number(distance) = args.get(0).unwrap() {
                    Command::Forward(*distance)
                } else {
                    panic!("'fd' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "forward",
            1,
            Box::new(|args| {
                if let Value::Number(distance) = args.get(0).unwrap() {
                    Command::Forward(*distance)
                } else {
                    panic!("'forward' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "bk",
            1,
            Box::new(|args| {
                if let Value::Number(distance) = args.get(0).unwrap() {
                    Command::Backward(*distance)
                } else {
                    panic!("'bk' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "back",
            1,
            Box::new(|args| {
                if let Value::Number(distance) = args.get(0).unwrap() {
                    Command::Backward(*distance)
                } else {
                    panic!("'back' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "lt",
            1,
            Box::new(|args| {
                if let Value::Number(angle) = args.get(0).unwrap() {
                    Command::Left(*angle)
                } else {
                    panic!("'lt' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "left",
            1,
            Box::new(|args| {
                if let Value::Number(angle) = args.get(0).unwrap() {
                    Command::Left(*angle)
                } else {
                    panic!("'left' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "rt",
            1,
            Box::new(|args| {
                if let Value::Number(angle) = args.get(0).unwrap() {
                    Command::Right(*angle)
                } else {
                    panic!("'rt' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "right",
            1,
            Box::new(|args| {
                if let Value::Number(angle) = args.get(0).unwrap() {
                    Command::Right(*angle)
                } else {
                    panic!("'right' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "repeat",
            2,
            Box::new(|args| {
                if let (Value::Number(reps), Value::List(body)) =
                    (args.get(0).unwrap(), args.get(1).unwrap())
                {
                    Command::Repeat(*reps, body.to_vec())
                } else {
                    panic!("'repeat' wymaga liczby i listy jako argumentów.")
                }
            }),
        ),
        Builtin::new("repcount", 0, Box::new(|_args| Command::RepCount)),
        Builtin::new(
            "label",
            1,
            Box::new(|args| {
                if let Some(v) = args.get(0) {
                    Command::Label(format!("{v}"))
                } else {
                    panic!("'label' wymaga napisu jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "setlabelheight",
            1,
            Box::new(|args| {
                if let Value::Number(height) = args.get(0).unwrap() {
                    Command::SetLabelHeight(*height)
                } else {
                    panic!("'setlabelheight' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new("clearscreen", 0, Box::new(|_args| Command::ClearScreen)),
        Builtin::new("window", 0, Box::new(|_args| Command::Window)),
        Builtin::new("wrap", 0, Box::new(|_args| Command::Wrap)),
        Builtin::new("pu", 0, Box::new(|_args| Command::PenUp)),
        Builtin::new("penup", 0, Box::new(|_args| Command::PenUp)),
        Builtin::new("pd", 0, Box::new(|_args| Command::PenDown)),
        Builtin::new("pendown", 0, Box::new(|_args| Command::PenDown)),
        Builtin::new(
            "wait",
            1,
            Box::new(|args| {
                if let Value::Number(delay) = args.get(0).unwrap() {
                    Command::Wait(*delay)
                } else {
                    panic!("'wait' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "setcolor",
            1,
            Box::new(|args| {
                if let Value::String(color) = args.get(0).unwrap() {
                    Command::SetColor(color.to_string())
                } else {
                    panic!("'setcolor' wymaga napisu jako argumentu. Dostał: {args:#?}")
                }
            }),
        ),
        Builtin::new(
            "setturtle",
            1,
            Box::new(|args| {
                if let Value::Number(turtle) = args.get(0).unwrap() {
                    Command::SetTurtle(turtle.trunc() as usize)
                } else {
                    panic!("'setturtle' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new(
            "pick",
            1,
            Box::new(|args| {
                if let Value::List(choices) = args.get(0).unwrap() {
                    Command::Pick(choices.to_vec())
                } else {
                    panic!("'pick' wymaga listy jako argumentu. Dostał: {args:#?}")
                }
            }),
        ),
        Builtin::new(
            "random",
            1,
            Box::new(|args| {
                if let Value::Number(range) = args.get(0).unwrap() {
                    Command::Random(*range)
                } else {
                    panic!("'random' wymaga liczby jako argumentu.")
                }
            }),
        ),
        Builtin::new("hideturtle", 0, Box::new(|_| Command::HideTurtle)),
        Builtin::new("showturtle", 0, Box::new(|_| Command::ShowTurtle)),
    ];

    let parsed = parse(&tokens, &keywords, &mut HashMap::new(), &builtins);

    let mut manager = Manager::new();

    eval(
        &parsed,
        &mut HashMap::new(),
        &mut HashMap::new(),
        &builtins,
        &mut manager,
    );

    manager.save(format!("{}.svg", result_filename));

    Ok(())
}

struct Builtin {
    name: String,
    argc: usize,
    body: Box<dyn Fn(Vec<Value>) -> Command>,
}

impl Builtin {
    fn new(name: &str, argc: usize, body: Box<dyn Fn(Vec<Value>) -> Command>) -> Self {
        Builtin {
            name: name.to_string(),
            argc,
            body,
        }
    }
}

#[derive(Debug, PartialEq, Clone)]
enum Keyword {
    To,
    End,
    If,
    Stop,
    Red,
    Orange,
    Yellow,
    Green,
    Blue,
    Violet,
}

#[derive(Debug, PartialEq, Clone)]
enum Token {
    Number(f64),
    String(String),
    Variable(String),
    ListStart,
    ListEnd,
    Procedure(String),
    Operator(Operator),
    Keyword(Keyword),
}

#[derive(Debug, Clone, PartialEq)]
pub enum Expr {
    Number(f64),
    String(String),
    Variable(String),
    List(Vec<Expr>),
    Operator(Operator),
    Operation(Box<Expr>, Operator, Box<Expr>),
    Assignment(String, Box<Expr>),
    ProcedureName(String),
    Procedure(String, Vec<String>, Vec<Expr>),
    ProcedureCall(String, Vec<Expr>),
    Command(Command),
    IfStart,
    If(Box<Expr>, Box<Expr>),
    Stop,
}

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Operator {
    Asterisk,
    Slash,
    Percent,
    Plus,
    Minus,
    LessThan,
    GreaterThan,
    Equals,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    Void,
    Bool(bool),
    Number(f64),
    String(String),
    Stop,
    List(Vec<Expr>),
    Expr(Expr),
    Command(Command),
}

impl std::fmt::Display for Value {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Value::Void => write!(f, "()"),
            Value::Bool(b) => write!(f, "{b}"),
            Value::Number(n) => write!(f, "{n}"),
            Value::String(s) => write!(f, "{s}"),
            Value::Stop => write!(f, "stop"),
            Value::List(body) => write!(f, "{body:?}"),
            Value::Expr(e) => write!(f, "{e:?}"),
            Value::Command(c) => write!(f, "{c:?}")
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum Command {
    Forward(f64),
    Backward(f64),
    Left(f64),
    Right(f64),
    PenUp,
    PenDown,
    ClearScreen,
    Repeat(f64, Vec<Expr>),
    RepCount,
    NextIter,
    SetColor(String),
    Pick(Vec<Expr>),
    Label(String),
    SetLabelHeight(f64),
    Window,
    Wrap,
    ShowTurtle,
    HideTurtle,
    SetTurtle(usize),
    Wait(f64),
    Random(f64),
}

fn split(source: &str) -> Vec<String> {
    let mut tokens: Vec<_> = Vec::new();

    let mut token = String::new();
    for ch in source.chars() {
        if "[](){}*/%<>=".contains(ch) {
            tokens.push(token.clone());
            tokens.push(ch.to_string());
            token.clear();
        } else if ch.is_whitespace() {
            tokens.push(token.clone());
            token.clear();
        } else {
            token.push(ch);
        }
    }
    tokens.push(token);

    tokens.into_iter().filter(|s| !s.is_empty()).collect()
}

fn tokenize(source: &str, keywords: &HashMap<&str, Keyword>) -> Vec<Token> {
    let split = split(source);
    let mut iter = split.iter().peekable();
    let mut tokens: Vec<Token> = Vec::new();
    while let Some(token) = iter.next() {
        if token.starts_with(':') {
            tokens.push(Token::Variable(token.to_string()));
        } else if token.starts_with(|ch| "\"\'".contains(ch)) {
            tokens.push(Token::String(token[1..].to_string()));
        } else if let Ok(n) = token.parse() {
            tokens.push(Token::Number(n));
        } else if token == "[" {
            tokens.push(Token::ListStart);
        } else if token == "]" {
            tokens.push(Token::ListEnd);
        } else if let Some(k) = keywords.get(&token[..]) {
            tokens.push(Token::Keyword(k.clone()));
        } else if "+-*/%<>=".contains(token) {
            tokens.push(Token::Operator(match &token[..] {
                "+" => Operator::Plus,
                "-" => Operator::Minus,
                "*" => Operator::Asterisk,
                "/" => Operator::Slash,
                "%" => Operator::Percent,
                "<" => Operator::LessThan,
                ">" => Operator::GreaterThan,
                "=" => Operator::Equals,
                _ => panic!("Nieznany operator."),
            }));
        } else {
            tokens.push(Token::Procedure(token.to_string()));
        }
    }

    tokens
}

fn fold_operations(
    exprs: &mut Vec<Expr>,
    token_iter: &mut Peekable<std::slice::Iter<'_, Token>>,
) -> bool {
    let mut len = exprs.len();
    if len < 3 {
        return false;
    }

    if let [left, Expr::Operator(op), right] = &exprs.get(len - 3..).unwrap().to_owned()[..] {
        let left_ok = matches!(
            left,
            Expr::Number(_)
                | Expr::Variable(_)
                | Expr::Operation(_, _, _)
                | Expr::ProcedureCall(_, _)
        );
        let right_ok = matches!(
            right,
            Expr::Number(_)
                | Expr::Variable(_)
                | Expr::Operation(_, _, _)
                | Expr::ProcedureCall(_, _)
        );

        if !left_ok || !right_ok {
            return false;
        }

        if match op {
            Operator::Asterisk | Operator::Slash | Operator::Percent => true,
            Operator::Plus | Operator::Minus => {
                !matches!(token_iter.peek(), Some(Token::Operator(op)) if matches!(op, Operator::Asterisk | Operator::Slash | Operator::Percent))
            }
            Operator::LessThan | Operator::GreaterThan => {
                !matches!(token_iter.peek(), Some(Token::Operator(op)) if matches!(op, Operator::Asterisk | Operator::Slash | Operator::Percent | Operator::Plus | Operator::Minus))
            }
            Operator::Equals => {
                if !matches!(token_iter.peek(), Some(Token::Operator(op)) if !matches!(op, Operator::Equals))
                {
                    if let Expr::Variable(name) = left {
                        exprs.drain(len - 3..).for_each(drop);
                        exprs.push(Expr::Assignment(
                            name.to_string(),
                            Box::new(right.to_owned()),
                        ));
                        len = exprs.len();
                    }
                }
                false
            }
        } {
            exprs.drain(len - 3..).for_each(drop);
            exprs.push(Expr::Operation(
                Box::new(left.to_owned()),
                *op,
                Box::new(right.to_owned()),
            ));
            return true;
        } else {
            return false;
        }
    };

    return false;
}

fn fold_procedures(
    exprs: &mut Vec<Expr>,
    token_iter: &mut Peekable<std::slice::Iter<'_, Token>>,
    procedures: &mut HashMap<String, usize>,
    builtins: &Vec<Builtin>,
) -> bool {
    let len = exprs.len();

    let mut reversed_tail = Vec::new();
    let clone = exprs.clone();
    let mut rev_iter = clone.iter().rev().cloned().peekable();
    while matches!(rev_iter.peek(), Some(e) if !matches!(e, Expr::ProcedureName(_))) {
        reversed_tail.push(rev_iter.next().unwrap());
    }

    loop {
        match rev_iter.peek() {
            Some(Expr::ProcedureName(_)) => break,
            None => return false,
            _ => reversed_tail.push(rev_iter.next().unwrap()),
        };
    }

    if let Some(Expr::ProcedureName(name)) = rev_iter.next() {
        let argc = match procedures.get(&name) {
            Some(argc) => argc.to_owned(),
            None => match builtins.iter().find(|builtin| builtin.name == *name) {
                Some(builtin) => builtin.argc,
                None => panic!("Niezdefiniowana procedura: {name}"),
            },
        };

        if argc == 0 {
            *exprs.last_mut().unwrap() = Expr::ProcedureCall(name.to_string(), Vec::new());
            return true;
        }

        let args = reversed_tail.iter().cloned().rev().collect::<Vec<_>>();
        match token_iter.peek() {
            Some(Token::Operator(_)) => return false,
            _ if args.len() >= argc && !exprs.iter().any(|e| matches!(e, Expr::Operator(_))) => {
                exprs.drain(len - argc - 1..).for_each(drop);
                exprs.push(Expr::ProcedureCall(
                    name.to_string(),
                    args.iter().map(|e| e.to_owned()).collect::<Vec<_>>(),
                ));

                return true;
            }
            _ => return false,
        }
    } else {
        return false;
    }
}

fn fold_ifs(exprs: &mut Vec<Expr>) -> bool {
    let len = exprs.len();

    if exprs.len() >= 3 {
        // Fold "if"
        if let [Expr::IfStart, condition, body] = &exprs.get(len - 3..).unwrap().to_owned()[..] {
            if matches!(condition, Expr::Operation(_, _, _))
                && !matches!(body, Expr::ProcedureName(_))
            {
                exprs.drain(len - 3..).for_each(drop);
                exprs.push(Expr::If(
                    Box::new(condition.clone()),
                    Box::new(body.clone()),
                ));

                return true;
            }
        }
    }

    return false;
}

fn parse(
    tokens: &[Token],
    keywords: &HashMap<&str, Keyword>,
    procedures: &mut HashMap<String, usize>,
    builtins: &Vec<Builtin>,
) -> Vec<Expr> {
    parse_iter(
        &mut tokens.iter().peekable(),
        keywords,
        procedures,
        builtins,
    )
}

fn parse_iter(
    token_iter: &mut Peekable<std::slice::Iter<'_, Token>>,
    keywords: &HashMap<&str, Keyword>,
    procedures: &mut HashMap<String, usize>,
    builtins: &Vec<Builtin>,
) -> Vec<Expr> {
    let mut exprs = vec![];
    while let Some(token) = token_iter.next() {
        match token {
            Token::Number(n) => exprs.push(Expr::Number(*n)),
            Token::String(str) => exprs.push(Expr::String(str.to_string())),
            Token::Variable(name) => exprs.push(Expr::Variable(name.to_string())),
            Token::ListStart => exprs.push(Expr::List(parse_iter(
                token_iter, keywords, procedures, builtins,
            ))),
            Token::ListEnd => return exprs,
            Token::Procedure(name) => exprs.push(Expr::ProcedureName(name.to_string())),
            Token::Operator(op) => exprs.push(Expr::Operator(*op)),
            Token::Keyword(keyword) => match keyword {
                Keyword::To => {
                    if let Some(Token::Procedure(name)) =
                        token_iter.next_if(|&t| matches!(t, Token::Procedure(_)))
                    {
                        let mut params = Vec::new();
                        while let Some(Token::Variable(param)) =
                            token_iter.next_if(|&t| matches!(t, Token::Variable(_)))
                        {
                            params.push(param.to_string());
                        }
                        procedures.insert(name.to_string(), params.len());
                        let body = parse_iter(token_iter, keywords, procedures, builtins);
                        exprs.push(Expr::Procedure(name.to_string(), params, body));
                    } else {
                        panic!("Po słowie kluczowym \"to\" powinna pojawić się nazwa definiowanej procedury.");
                    }
                }
                Keyword::End => return exprs,
                Keyword::If => exprs.push(Expr::IfStart),
                Keyword::Stop => exprs.push(Expr::Stop),
                Keyword::Red => exprs.push(Expr::String("#FF0000".to_string())),
                Keyword::Orange => exprs.push(Expr::String("#FF8000".to_string())),
                Keyword::Yellow => exprs.push(Expr::String("#FFFF00".to_string())),
                Keyword::Green => exprs.push(Expr::String("#00FF00".to_string())),
                Keyword::Blue => exprs.push(Expr::String("#0000FF".to_string())),
                Keyword::Violet => exprs.push(Expr::String("#FF00FF".to_string())),
            },
        }

        while fold_operations(&mut exprs, token_iter)
            || fold_procedures(&mut exprs, token_iter, procedures, builtins)
            || fold_ifs(&mut exprs)
        {}
    }

    exprs
}

fn eval_single(
    expr: &Expr,
    env: &mut HashMap<String, Value>,
    procedures: &mut HashMap<String, (Vec<String>, Vec<Expr>)>,
    builtins: &Vec<Builtin>,
    manager: &mut Manager,
) -> Value {
    match expr {
        Expr::Number(n) => Value::Number(*n),
        Expr::Variable(name) => env.get(name).unwrap_or(&Value::Number(0f64)).clone(),
        Expr::String(s) => Value::String(s.to_string()),
        Expr::Operator(op) => panic!("Niezłożony operator: {op:?}"),
        Expr::Operation(left, op, right) => {
            let mut value1 = eval_single(left, env, procedures, builtins, manager);
            let mut value2 = eval_single(right, env, procedures, builtins, manager);

            if let Value::Command(Command::RepCount) = value1 {
                value1 = manager.send_command(&Command::RepCount);
            }

            if let Value::Command(Command::RepCount) = value2 {
                value2 = manager.send_command(&Command::RepCount);
            }

            if let (Value::Number(n1), Value::Number(n2)) = (value1, value2) {
                match op {
                    Operator::Plus => Value::Number(n1 + n2),
                    Operator::Minus => Value::Number(n1 - n2),
                    Operator::Asterisk => Value::Number(n1 * n2),
                    Operator::Slash => Value::Number(n1 / n2),
                    Operator::Percent => Value::Number(n1 % n2),
                    Operator::LessThan => Value::Bool(n1 < n2),
                    Operator::GreaterThan => Value::Bool(n1 > n2),
                    Operator::Equals => Value::Number(n2),
                }
            } else {
                panic!("Próba wykonania operacji na czymś innym niż liczby.");
            }
        }
        Expr::Procedure(name, params, body) => {
            procedures.insert(name.to_string(), (params.to_vec(), body.to_vec()));
            Value::Void
        }
        Expr::ProcedureCall(name, args) => {
            let mut env = env.clone();
            if let Some((params, body)) = procedures.clone().get(name) {
                let mut new_env = env.clone();
                for (param, arg) in params.iter().zip(
                    args.iter()
                        .map(|arg| eval_single(arg, &mut env, procedures, builtins, manager)),
                ) {
                    new_env.insert(param.to_string(), arg);
                }
                match eval(body, &mut new_env, procedures, builtins, manager) {
                    Value::Stop => Value::Void,
                    other => other,
                }
            } else {
                match builtins.iter().find(|&builtin| &builtin.name == name) {
                    Some(builtin) => {
                        let evaluated_args = args
                            .iter()
                            .map(|arg| {
                                if let Expr::List(body) = arg {
                                    Value::List(body.to_vec())
                                } else {
                                    let e =
                                        eval_single(arg, &mut env, procedures, builtins, manager);
                                    if let Value::Expr(e) = e {
                                        eval_single(&e, &mut env, procedures, builtins, manager)
                                    } else {
                                        e
                                    }
                                }
                            })
                            .collect();
                        let result = (builtin.body)(evaluated_args);

                        let response = manager.send_command(&result);
                        if let Value::List(exprs) = &response {
                            exprs.iter().for_each(|expr| {
                                eval_single(expr, &mut env, procedures, builtins, manager);
                            })
                        }
                        response
                    }
                    None => panic!("Procedura '{name}' nie została zdefiniowana."),
                }
            }
        }
        Expr::List(body) => {
            let result_body = body
                .iter()
                .map(|expr| eval_single(expr, env, procedures, builtins, manager))
                .collect::<Vec<_>>();
            if result_body.contains(&Value::Stop) {
                Value::Stop
            } else {
                Value::List(body.to_vec())
            }
        }
        Expr::IfStart => todo!(),
        Expr::If(condition, body) => {
            if let Value::Bool(true) = eval_single(condition, env, procedures, builtins, manager) {
                eval_single(body, env, procedures, builtins, manager)
            } else {
                Value::Void
            }
        }
        Expr::Command(command) => manager.send_command(command),
        Expr::Assignment(name, value) => {
            let new_value = eval_single(value, env, procedures, builtins, manager);
            env.insert(name.to_string(), new_value);
            Value::Void
        }
        Expr::Stop => Value::Stop,
        Expr::ProcedureName(name) => panic!("Niezłożona nazwa procedury: {name}"),
    }
}

fn eval(
    exprs: &Vec<Expr>,
    env: &mut HashMap<String, Value>,
    proc: &mut HashMap<String, (Vec<String>, Vec<Expr>)>,
    builtins: &Vec<Builtin>,
    manager: &mut Manager,
) -> Value {
    let mut res = Value::Void;
    for expr in exprs {
        res = eval_single(expr, env, proc, builtins, manager);
        if let Value::Stop = res {
            return Value::Stop;
        }
    }
    res
}
