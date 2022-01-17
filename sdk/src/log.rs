#![cfg(feature = "program")]

pub use paychains_program::log::*;

#[macro_export]
#[deprecated(
    since = "1.4.3",
    note = "Please use `paychains_program::log::info` instead"
)]
macro_rules! info {
    ($msg:expr) => {
        $crate::log::pay_log($msg)
    };
}
