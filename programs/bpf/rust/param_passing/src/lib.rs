//! Example Rust-based BPF program tests loop iteration

extern crate paychains_program;
use paychains_bpf_rust_param_passing_dep::{Data, TestDep};
use paychains_program::{custom_panic_default, entrypoint::SUCCESS, log::pay_log_64};

#[no_mangle]
pub extern "C" fn entrypoint(_input: *mut u8) -> u64 {
    let array = [0xA, 0xB, 0xC, 0xD, 0xE, 0xF];
    let data = Data {
        twentyone: 21u64,
        twentytwo: 22u64,
        twentythree: 23u64,
        twentyfour: 24u64,
        twentyfive: 25u32,
        array: &array,
    };

    let test_dep = TestDep::new(&data, 1, 2, 3, 4, 5);
    pay_log_64(0, 0, 0, 0, test_dep.thirty as u64);
    assert!(test_dep.thirty == 30);

    SUCCESS
}

custom_panic_default!();

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_entrypoint() {
        assert_eq!(SUCCESS, entrypoint(std::ptr::null_mut()));
    }
}
