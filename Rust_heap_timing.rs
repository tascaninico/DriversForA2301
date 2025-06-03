use std::time::Instant;

fn main() {
    let start = Instant::now();
    let data = Box::new([0u8; 1000]);
    let after_alloc = Instant::now();

    let sum: u8 = data.iter().sum();
    println!("Sum = {}", sum);

    let before_free = Instant::now();
    drop(data);
    let after_free = Instant::now();

    println!("Box::new took {:?} ns", after_alloc.duration_since(start).as_nanos());
    println!("drop(Box) took {:?} ns", after_free.duration_since(before_free).as_nanos());
}
