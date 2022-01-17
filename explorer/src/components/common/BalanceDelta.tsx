import React from "react";
import { BigNumber } from "bignumber.js";
import { PayBalance } from "utils";

export function BalanceDelta({
  delta,
  isPay = false,
}: {
  delta: BigNumber;
  isPay?: boolean;
}) {
  let sols;

  if (isPay) {
    sols = <PayBalance lamports={delta.toNumber()} />;
  }

  if (delta.gt(0)) {
    return (
      <span className="badge bg-success-soft">
        +{isPay ? sols : delta.toString()}
      </span>
    );
  } else if (delta.lt(0)) {
    return (
      <span className="badge bg-warning-soft">
        {isPay ? <>-{sols}</> : delta.toString()}
      </span>
    );
  }

  return <span className="badge bg-secondary-soft">0</span>;
}
