1)Il logger e' implementato come singleton.

La prima chiamata avveniva nel expire del timer.

Risulta che in una callback di un ISR non e' possibile creare il singleton.

Vale per ogni allocazione dinamica della memoria?

2)Per debuggare dopo aver "provato" un altro software ho dovuto usare il tasto Debug in actions. Perche'?