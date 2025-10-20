mod minigrep;

use std::{env, process};
use minigrep::{Config};

// set IGNORE_CASE=1
// cargo run -- the Cargo.toml
// cargo run -- to Cargo.toml > output.txt
fn main() {
    //dbg!(args);
    let config = Config::build(env::args()).unwrap_or_else(|err|{
        eprintln!("Problem parsing args: {}", err);
        process::exit(1)
    });
    println!("Query is {}", config.query);
    println!("Filepath is {}", config.file_path);
    if let Err(e) = minigrep::run(config) {
        eprintln!("Application error: {}", e);
        process::exit(1)
    }
}