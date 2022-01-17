import React from "react";
import { PayChainsClusterStatsProvider } from "./paychainsClusterStats";

type Props = { children: React.ReactNode };
export function StatsProvider({ children }: Props) {
  return <PayChainsClusterStatsProvider>{children}</PayChainsClusterStatsProvider>;
}
