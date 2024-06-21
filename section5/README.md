<script type="text/javascript" async src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.7/MathJax.js?config=TeX-MML-AM_CHTML">
</script>
<script type="text/x-mathjax-config">
 MathJax.Hub.Config({
 tex2jax: {
 inlineMath: [['$', '$'] ],
 displayMath: [ ['$$','$$'], ["\\[","\\]"] ]
 }
 });
</script>


# エラトステネスの篩
エラトステネスの篩とは，整数n以下の素数を判定するアルゴリズムである。
疑似コードは以下のようになる。
1. $2,3,4...,n$ の配列を作る。最初すべての値はマークされていない。
1. $k$ に $2$ を代入する。
1. 以下を $k^2>n$ である限り繰り返す。
   1. $k^2$ から $n$ までのkの倍数をマークする。
   1. $k$ よりも大きくまだマークされていない数の中で最小のものを選び， $k$ に代入する。
1. マークされていない数が素数である。


## 並列化
並列化のカギはステップ3。1にある。
$j \mod k \equiv 0$ ならば， $j$ は $k$ の倍数であり，マークされる。
一度のループで $k$ の新たな値を決めるためにreductionが必要である。
そして， $k$ をすべてのプロセスに送信するのにbroadcastが必要である。
どのように領域分割を行うかを考える必要がある。


## データ分割
### インターリーブデータ分割
以下のような __インターリーブデータ分割__ を考える。
$$
\begin{cases}
\text{process 0: } &2, 2+p, 2+2p, \cdots \\
\text{process 1: } &3, 3+p, 3+2p, \cdots \\
                   &\vdots
\end{cases}
$$
ただし，$p$ は1プロセス当たりのサイズである。
この方式の利点は，あるインデックス $i$ が与えられたときに，どのプロセスが扱うか決定することが容易であることである。
欠点はロードバランスが非常に悪いことである。
例えば，上の２つのプロセスが $2$ の倍数をマークする場合，プロセス0は$\lceil(n-1)/2\rceil$ 要素をマークするが，プロセス1は一つもマークしない。
さらに，ステップ3.2で次の $k$ を探すために，reduction/broadcastが必要となる。


### ブロックデータ分割
代わりの方法として， __ブロックデータ分割__ がある。
これは，配列を連続する $p$ 個のブロックごとに分割する方法である。
$$
\begin{cases}
\text{process 0: } &2, 3, \cdots, p+1 \\
\text{process 1: } &2+p, 3+p, \cdots, 2p+1 \\
                   &\vdots
\end{cases}
$$
プロセス $i$ は $\lfloor in/p\rfloor$ から $\lfloor (i+1)n/p\rfloor-1$ を扱う。