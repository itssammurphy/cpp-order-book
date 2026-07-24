from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


OUTPUT_DIR = Path("output")

snapshots = pd.read_csv(OUTPUT_DIR / "booksnapshots.csv")
trades = pd.read_csv(OUTPUT_DIR / "trades.csv")


def plot_prices() -> None:
    plt.figure(figsize=(11, 6))

    plt.plot(
        snapshots["step"],
        snapshots["fair_value"],
        label="Fair value",
    )

    plt.plot(
        snapshots["step"],
        snapshots["best_bid"],
        label="Best bid"
    )

    plt.plot(
        snapshots["step"],
        snapshots["best_ask"],
        label="Best ask"
    )

    if not trades.empty:
        buy_trades = trades[trades["aggressor"] == "BUY"]
        sell_trades = trades[trades["aggressor"] == "SELL"]

        plt.scatter(
            buy_trades["step"],
            buy_trades["price"],
            marker="^",
            label="Buy-aggressor trade",
        )

        plt.scatter(
            sell_trades["step"],
            sell_trades["price"],
            marker="v",
            label="Sell-aggressor trade",
        )

    plt.title("Order book prices")
    plt.xlabel("Simulation step")
    plt.ylabel("Price")
    plt.legend()
    plt.tight_layout()
    plt.show()


def plot_spread() -> None:
    plt.figure(figsize=(11, 5))

    plt.plot(
        snapshots["step"],
        snapshots["spread"],
    )

    plt.title("Bid-ask spread")
    plt.xlabel("Simulation step")
    plt.ylabel("Spread")
    plt.tight_layout()
    plt.show()


def plot_volume() -> None:
    plt.figure(figsize=(11, 5))

    plt.plot(
        snapshots["step"],
        snapshots["bid_volume"],
        label="Bid volume",
    )

    plt.plot(
        snapshots["step"],
        snapshots["ask_volume"],
        label="Ask volume",
    )

    plt.title("Resting book volume")
    plt.xlabel("Simulation step")
    plt.ylabel("Quantity")
    plt.legend()
    plt.tight_layout()
    plt.show()


plot_prices()
plot_spread()
plot_volume()
