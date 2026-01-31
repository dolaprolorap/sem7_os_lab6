import threading
import requests
import tkinter as tk
from tkinter import messagebox
import numpy as np
import matplotlib.pyplot as plt


URL = "http://localhost:8080/data?start=0&finish=1769863698"


def fetch_data():
    response = requests.get(URL, timeout=5)
    response.raise_for_status()
    return response.json()


def plot_data(data):
    plt.figure()
    plt.plot(np.arange(len(data)), data, marker="o")
    plt.ylabel("Температура")
    plt.title("Данные с сервера")
    plt.grid(True)
    plt.show()


def on_button_click():
    def worker():
        try:
            data = fetch_data()
            plot_data(data)
        except Exception as e:
            messagebox.showerror("Ошибка", str(e))

    threading.Thread(target=worker, daemon=True).start()


def main():
    root = tk.Tk()
    root.title("HTTP клиент")

    btn = tk.Button(
        root,
        text="Получить данные и построить график",
        command=on_button_click,
        width=40,
        height=2
    )
    btn.pack(padx=20, pady=20)

    root.mainloop()


if __name__ == "__main__":
    main()
