
trait Pilot {
    fn fly(&self);
    fn name();
}

trait Wizard {
    fn fly(&self);
    fn name();
}

struct Human; // 父类

impl Pilot for Human {
    fn fly(&self){ // 子类实现
       println!("Pilot fly");
    }
    fn name(){ // 子类实现
       println!("Pilot");
    }
}

impl Wizard for Human {
    fn fly(&self){ // 子类实现
       println!("Wizard fly");
    }
    fn name(){ // 子类实现
       println!("Wizard");
    }
}

impl Human {
    fn fly(&self){ //父类方法
       println!("Human fly");
    }
    fn name(){ // 子类实现
       println!("Human");
    }
}

fn main() {
    let v = vec![1,2,3,4];
    // iter 返回的类型是&i32 ， 
    //`filter`闭包接收的是当前迭代器元素的引用
    let a: Vec<_> = v.iter().filter(|x:  & &i32 | { /* %运算符会自动解引用所以不写**x */ *x % 2 == 0 }).map(|x: &i32| { x * 2 }).collect();
    let b: Vec<_> = v.iter().map(|x: &i32| { x * 2 }).filter(|x: &i32 | { *x % 2 == 0 }).collect();
    println!("{a:?}, {b:?}, {v:?}");

    let v = vec!["1", "2", "3", "4"];
    
    // 方案A：先过滤后映射
    let a: Vec<_> = v.iter()
        .filter(|& &s| s.parse::<i32>().unwrap() % 2 == 0)  // 过滤偶数
        .map(|&s| (s.parse::<i32>().unwrap() * 2).to_string()) // 乘以2后转字符串
        .collect();
    
    // 方案B：先映射后过滤
    let b: Vec<_> = v.iter()
        .map(|&s| s.parse::<i32>().unwrap() * 2)  // 先乘以2
        .filter(|&x| x % 2 == 0)                  // 过滤偶数（总是成立）
        .map(|x| x.to_string())                   // 转字符串
        .collect();

    unsafe {
        let mut v = vec![1,2,3,4];
        let r = &mut v[..];
        let (a, b) = split_at_mid(r, 3);
        println!("{:?}", a);
        println!("{:?}", b);
    }

    let person = Human;
    person.fly(); 		 //输出Human fly，父类调用
    Pilot::fly(&person); //输出Pilot fly，多态子类调用
    Wizard::fly(&person); //输出Wizard fly，多态子类调用
    Human::name();
    <Human as Wizard>::name();


}
use std::slice;
fn split_at_mid(values: &mut [i32], mid: usize) -> ( &mut [i32], &mut [i32])
{
    /*
	let len = values.len();
    
    assert!(mid <= len);
    
    (&mut values[..mid], &mut values[mid..]) //返回了两个不同部分的借用，但是rust不知道，它认为我们解借用了values两次
    */
    //通过unsafe实现
    let len = values.len();
    let ptr = values.as_mut_ptr(); // 常量指针
    assert!(mid <= len);
    
    unsafe {
		(
            slice::from_raw_parts_mut(ptr, mid),
        	slice::from_raw_parts_mut(ptr.add(mid), len - mid),
        )
	}
}