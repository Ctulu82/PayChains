import BN from "bn.js";
import {
  HumanizeDuration,
  HumanizeDurationLanguage,
} from "humanize-duration-ts";
import { PublicKey } from "@paychains/web3.js";

// Switch to web3 constant when web3 updates superstruct
export const LAMPORTS_PER_PAY = 1000000000;

export const NUM_TICKS_PER_SECOND = 160;
export const DEFAULT_TICKS_PER_SLOT = 64;
export const NUM_SLOTS_PER_SECOND =
  NUM_TICKS_PER_SECOND / DEFAULT_TICKS_PER_SLOT;
export const MS_PER_SLOT = 1000 / NUM_SLOTS_PER_SECOND;

export function assertUnreachable(x: never): never {
  throw new Error("Unreachable!");
}

export function normalizeTokenAmount(
  raw: string | number,
  decimals: number
): number {
  let rawTokens: number;
  if (typeof raw === "string") rawTokens = parseInt(raw);
  else rawTokens = raw;
  return rawTokens / Math.pow(10, decimals);
}

export function lamportsToPay(lamports: number | BN): number {
  if (typeof lamports === "number") {
    return Math.abs(lamports) / LAMPORTS_PER_PAY;
  }

  let signMultiplier = 1;
  if (lamports.isNeg()) {
    signMultiplier = -1;
  }

  const absLamports = lamports.abs();
  const lamportsString = absLamports.toString(10).padStart(10, "0");
  const splitIndex = lamportsString.length - 9;
  const solString =
    lamportsString.slice(0, splitIndex) +
    "." +
    lamportsString.slice(splitIndex);
  return signMultiplier * parseFloat(solString);
}

export function lamportsToPayString(
  lamports: number | BN,
  maximumFractionDigits: number = 9
): string {
  const sol = lamportsToPay(lamports);
  return new Intl.NumberFormat("en-US", { maximumFractionDigits }).format(pay);
}

export function PayBalance({
  lamports,
  maximumFractionDigits = 9,
}: {
  lamports: number | BN;
  maximumFractionDigits?: number;
}) {
  return (
    <span>
      ◎
      <span className="font-monospace">
        {lamportsToPayString(lamports, maximumFractionDigits)}
      </span>
    </span>
  );
}

const HUMANIZER = new HumanizeDuration(new HumanizeDurationLanguage());
HUMANIZER.setOptions({
  language: "short",
  spacer: "",
  delimiter: " ",
  round: true,
  units: ["d", "h", "m", "s"],
  largest: 3,
});
HUMANIZER.addLanguage("short", {
  y: () => "y",
  mo: () => "mo",
  w: () => "w",
  d: () => "d",
  h: () => "h",
  m: () => "m",
  s: () => "s",
  ms: () => "ms",
  decimal: ".",
});

export function slotsToHumanString(
  slots: number,
  slotTime = MS_PER_SLOT
): string {
  return HUMANIZER.humanize(slots * slotTime);
}

export function wrap(input: string, length: number): string {
  var result = [];
  while (input.length) {
    result.push(input.substr(0, length));
    input = input.substr(length);
  }
  return result.join("\n");
}

export function localStorageIsAvailable() {
  const test = "test";
  try {
    localStorage.setItem(test, test);
    localStorage.removeItem(test);
    return true;
  } catch (e) {
    return false;
  }
}

export function camelToTitleCase(str: string): string {
  const result = str.replace(/([A-Z])/g, " $1");
  return result.charAt(0).toUpperCase() + result.slice(1);
}

export function abbreviatedNumber(value: number, fixed = 1) {
  if (value < 1e3) return value;
  if (value >= 1e3 && value < 1e6) return +(value / 1e3).toFixed(fixed) + "K";
  if (value >= 1e6 && value < 1e9) return +(value / 1e6).toFixed(fixed) + "M";
  if (value >= 1e9 && value < 1e12) return +(value / 1e9).toFixed(fixed) + "B";
  if (value >= 1e12) return +(value / 1e12).toFixed(fixed) + "T";
}

export const pubkeyToString = (key: PublicKey | string = "") => {
  return typeof key === "string" ? key : key?.toBase58() || "";
};

export const getLast = (arr: string[]) => {
  if (arr.length <= 0) {
    return undefined;
  }

  return arr[arr.length - 1];
};
