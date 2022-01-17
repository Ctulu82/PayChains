import { expect } from "chai";
import { lamportsToPay, LAMPORTS_PER_PAY } from "utils";
import BN from "bn.js";

describe("lamportsToPay", () => {
  it("0 lamports", () => {
    expect(lamportsToPay(new BN(0))).to.eq(0.0);
  });

  it("1 lamport", () => {
    expect(lamportsToPay(new BN(1))).to.eq(0.000000001);
    expect(lamportsToPay(new BN(-1))).to.eq(-0.000000001);
  });

  it("1 PAY", () => {
    expect(lamportsToPay(new BN(LAMPORTS_PER_PAY))).to.eq(1.0);
    expect(lamportsToPay(new BN(-LAMPORTS_PER_PAY))).to.eq(-1.0);
  });

  it("u64::MAX lamports", () => {
    expect(lamportsToPay(new BN(2).pow(new BN(64)))).to.eq(
      18446744073.709551615
    );
    expect(lamportsToPay(new BN(2).pow(new BN(64)).neg())).to.eq(
      -18446744073.709551615
    );
  });
});
