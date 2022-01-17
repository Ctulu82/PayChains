#![allow(clippy::integer_arithmetic)]
/// There are 10^9 lamports in one PAY
pub const LAMPORTS_PER_PAY: u64 = 1_000_000_000;

/// Approximately convert fractional native tokens (lamports) into native tokens (PAY)
pub fn lamports_to_pay(lamports: u64) -> f64 {
    lamports as f64 / LAMPORTS_PER_PAY as f64
}

/// Approximately convert native tokens (PAY) into fractional native tokens (lamports)
pub fn pay_to_lamports(sol: f64) -> u64 {
    (sol * LAMPORTS_PER_PAY as f64) as u64
}

use std::fmt::{Debug, Display, Formatter, Result};
pub struct Pay(pub u64);

impl Pay {
    fn write_in_pay(&self, f: &mut Formatter) -> Result {
        write!(
            f,
            "◎{}.{:09}",
            self.0 / LAMPORTS_PER_PAY,
            self.0 % LAMPORTS_PER_PAY
        )
    }
}

impl Display for Pay {
    fn fmt(&self, f: &mut Formatter) -> Result {
        self.write_in_pay(f)
    }
}

impl Debug for Pay {
    fn fmt(&self, f: &mut Formatter) -> Result {
        self.write_in_pay(f)
    }
}
