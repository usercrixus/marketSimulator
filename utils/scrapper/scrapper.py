import requests
import json
import time

def fetch_last_agg_trades(symbol="BTCUSDT", limit=100):
    """
    Fetch exactly the last `limit` aggregated trades for `symbol`.
    Returns a list of dicts: [{"price": float, "qty": float, "time": int}, …].
    """
    url = "https://api.binance.com/api/v3/aggTrades"
    params = {
        "symbol": symbol,
        "limit":  limit
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

def fetch_top_of_book(symbol="BTCUSDT", depth_limit=10):
    """
    Fetch `depth_limit` levels of bids and asks (top of book) for `symbol`.
    Returns two lists of [price: float, quantity: float].
    """
    url = "https://api.binance.com/api/v3/depth"
    params = {"symbol": symbol, "limit": depth_limit}
    resp = requests.get(url, params=params, timeout=5)
    resp.raise_for_status()
    raw = resp.json()

    bids = [[float(p), float(q)] for p, q in raw.get("bids", [])[:depth_limit]]
    asks = [[float(p), float(q)] for p, q in raw.get("asks", [])[:depth_limit]]
    return bids, asks

def main():
    symbol = "BTCUSDT"
    depth_limit = 10   # we want the top 10 bids & top 10 asks

    # 1) Step: fetch the 100 most recent agg trades at once
    last_trades = fetch_last_agg_trades(symbol=symbol, limit=100)

    # Binance’s aggTrades come back sorted oldest→newest, so we’ll treat
    # last_trades[0] as “oldest of the last 100” and last_trades[-1] as “latest.” 
    # In other words, epoch 0 corresponds to last_trades[-100], epoch 99 to last_trades[-1].
    #
    # 2) Now, for each of those 100 trades, we’ll immediately fetch the top‐of‐book (10 bids, 10 asks)
    #    and record exactly one trade + its “new snapshot” of bids/asks.
    #
    #    Because the REST /aggTrades endpoint does not let us reconstruct the exact order book
    #    at each trade’s original timestamp, we are only capturing “best bids/asks as of the moment
    #    we loop here.” In practice that means your C++ backtest will replay “trade_i plus the book
    #    snapshot that followed just now.” It will not be *exactly* the order book at the original
    #    millisecond `time` of that trade, but in a live‐running script you’d run this Python immediately
    #    after retrieving aggTrades so the book is only a few ms out of sync. (If you need perfect
    #    historical book‐at‐trade‐time, you must use Binance’s websocket or a historical dump.)
    #
    snapshots = []  # this will become our JSON array of length 100

    for idx, trade in enumerate(last_trades):
        # 2a) Unpack this one trade’s price/qty
        trade_price = trade["price"]
        trade_qty   = trade["qty"]

        # 2b) Fetch top 10 bids & top 10 asks “right now”
        bids, asks = fetch_top_of_book(symbol, depth_limit=depth_limit)

        # 2c) Build a single‐trade snapshot
        record = {
            "bids":   bids,             # [[bid_price, bid_qty], … (10 entries)]
            "asks":   asks,             # [[ask_price, ask_qty], … (10 entries)]
            "trades": [
                { "price": trade_price, "qty": trade_qty }
            ]
        }
        snapshots.append(record)

        # (Optional) Print progress. Note that idx runs 0..99 here.
        print(f"[{idx+1:3d}/100] Recorded trade#{idx} "
              f"(price={trade_price:.2f}, qty={trade_qty:.6f}) "
              f"with top‐10 book snapshot.")

        # If you want a short pause between book fetches (to avoid hitting rate limits),
        # you can uncomment the next line. But at depth=10 and aggTrades=100, you should be safe
        # under Binance’s 1200 requests/minute rule if you do this at startup:
        # time.sleep(0.05)

    # 3) Now dump that list of 100 records to JSON exactly where your C++ expects it:
    with open("binance_snapshots_clean.json", "w") as f:
        json.dump(snapshots, f, indent=2)

    print("Wrote binance_snapshots_clean.json with 100 epochs.")

if __name__ == "__main__":
    main()
