use std::{env, error::Error, fs};
pub struct Config {
    pub query: String,
    pub file_path: String,
    pub ignore_case: bool,
}

impl Config {
    pub fn build(mut args: impl Iterator<Item = String>) -> Result<Config, &'static str>{
       
        args.next();
        let query = match args.next() {
            Some(arg) => arg,
            None => return Err("Didn't get a query string"),
        };
        let file_path = match args.next() {
            Some(arg) => arg,
            None => return Err("Didn't get a file name"),
        };
        Ok(Config {
            query,
            file_path,
            ignore_case: env::var("IGNORE_CASE").is_ok(),
        })
    }
}

pub fn run(config: Config) ->Result<(), Box<dyn Error>> {
    let contents = fs::read_to_string(config.file_path)?;

    let results =
        if !config.ignore_case { search(&config.query, &contents) }
        else {search(&config.query, &contents) };

    println!("Read file contents is \n{contents}");

    println!("Search results:");
    for line in results {
        println!("{line}");
    }
    Ok(())
}

pub fn search<'a>(word: &str, content: &'a str) -> Vec<&'a str> {
    contents
        .lines()
        .filter(|line| {line.contains(query)})
        .collect()
}

pub fn search_case_insensitive<'a>(word: &str, content: &'a str) -> Vec<&'a str> {
    let query = query.to_lowercase();
    contents
        .lines()
        .filter(|line| {line.to_lowercase().contains(&query)})
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn contains_one_word_test() {
        let query_word = "melody";
        let music_contents = "\
            原来从未忘记~~~
            melody yy
            脑海中的旋律转个不停
        ";
        assert_eq!(true, music_contents.contains(query_word));
    }

    #[test]
    fn search_word_test() {
        let query_word = "melody";
        let music_contents = 
"\
原来从未忘记~~~
melody yy
脑海中的旋律转个不停
";
        assert_eq!(vec!["melody"], search(query_word, music_contents));
        assert_eq!(vec!["melody"], search_case_insensitive(query_word, music_contents));
    }
}