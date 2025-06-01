import time
import requests
import json

def fetch_order_book(symbol="BTCUSDT", limit=10):
    """
    Fetch a single order‐book snapshot (top `limit` bids/asks)
    and return them as lists of [float_price, float_qty].
    """
    url = "https://api.binance.com/api/v3/depth"
    params = {"symbol": symbol, "limit": limit}
    resp = requests.get(url, params=params, timeout=5)
    resp.raise_for_status()
    raw = resp.json()

    bids = [[float(p), float(q)] for p, q in raw["bids"][:limit]]
    asks = [[float(p), float(q)] for p, q in raw["asks"][:limit]]

    return bids, asks

def fetch_trades_in_window(symbol="BTCUSDT", start_ts=None, end_ts=None, limit=500):
    """
    Fetch aggregated trades that occurred between start_ts and end_ts (ms since epoch).
    Returns a list of {price, qty, time} dicts.
    """
    url = "https://api.binance.com/api/v3/aggTrades"
    params = {
        "symbol": symbol,
        "startTime": start_ts,
        "endTime":   end_ts,
        "limit":     limit
    }
    resp = requests.get(url, params=params, timeout=5)
    resp.raise_for_status()
    raw = resp.json()

    trades = []
    for t in raw:
        trades.append({
            "price": float(t["p"]),
            "qty":   float(t["q"]),
            "time":  int(t["T"])
        })
    return trades

def main():
    symbol = "BTCUSDT"
    snapshots = []

    for i in range(100):  # change to 1000 if you like
        # 1) Record the current iteration index instead of timestamp
        iteration_index = i

        # 2) Grab the order‐book snapshot (top 10 levels)
        bids, asks = fetch_order_book(symbol, limit=10)

        # 3) Calculate a 5‐second window [now−5s, now]
        now_ms    = int(time.time() * 1000)
        window_ms = 5 * 1000
        start_ms  = now_ms - window_ms
        end_ms    = now_ms

        # 4) Fetch only the trades that occurred in that 5s window
        trades = fetch_trades_in_window(symbol, start_ts=start_ms, end_ts=end_ms, limit=500)

        # 5) Build a compact record
        record = {
            "iter":  iteration_index,
            "bids":  bids,
            "asks":  asks,
            "trades": trades
        }
        snapshots.append(record)

        print(f"[{i+1:3d}/100] Captured iteration {iteration_index}, window {start_ms}–{end_ms}")
        time.sleep(5)

    # 6) Write to disk as pretty JSON
    with open("binance_snapshots_clean.json", "w") as f:
        json.dump(snapshots, f, indent=2)

if __name__ == "__main__":
    main()
