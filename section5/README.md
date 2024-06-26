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
並列化のカギはステップ3.1にある。  
$j \mod k \equiv 0$ ならば， $j$ は $k$ の倍数であり，マークされる。  
一度のループで $k$ の新たな値を決めるために`reduction`が必要である。  
そして， $k$ をすべてのプロセスに送信するのに`broadcast`が必要である。  
どのように領域分割を行うかを考える必要がある。  


## データ分割
### インターリーブデータ分割
以下のような __インターリーブデータ分割__ を考える。
```math
\begin{cases}
\text{process 0: } &2, 2+p, 2+2p, \cdots \\
\text{process 1: } &3, 3+p, 3+2p, \cdots \\
                   &\vdots
\end{cases}
```
ただし, $p$ は1プロセス当たりのサイズである。  
この方式の利点は，あるインデックス $i$ が与えられたときに，どのプロセスが扱うか決定することが容易であることである。  
欠点はロードバランスが非常に悪いことである。  
例えば，上の２つのプロセスが $2$ の倍数をマークする場合，プロセス0は $\lceil(n-1)/2\rceil$ 要素をマークするが，プロセス1は一つもマークしない。  
さらに，ステップ3.2で次の $k$ を探すために，`reduction`/`broadcast`が必要となる。


### ブロックデータ分割
代わりの方法として， __ブロックデータ分割__ がある。
これは，配列を連続する $p$ 個のブロックごとに分割する方法である。
```math
\begin{cases}
\text{process 0: } &2, 3, \cdots, p+1 \\
\text{process 1: } &2+p, 3+p, \cdots, 2p+1 \\
                   &\vdots
\end{cases}
```
プロセス $i$ は $\lfloor in/p\rfloor$ から $\lfloor (i+1)n/p\rfloor-1$ を扱う。


## 性能評価
エラトステネスの篩の計算量は $\mathcal{O}(n\ln\ln n)$ である。
> 証明:  
> ある素数 $p$ によってふるい落とす回数は $n/p$ 回である。  
> したがって，  
> $$\sum_{p<\sqrt{n}}\frac{n}{p} = n\left(\frac{1}{2}+\frac{1}{3}+\frac{1}{5}+\cdots\right)$$
> ここで，素数の逆数和の発散速度はおおよそ $\ln\ln \sqrt{n}\sim\ln\ln n+\ln(1/2)$ であるため，  
> エラトステネスの篩の計算量は， $\mathcal{O}(n\ln\ln n)$ となる。

1コアでのプログラムの実行時間 $T_1$ から， $\chi$ を 
$$\chi = \frac{T_1}{n\ln\ln n}$$ 
とする。
プロセス間の通信にかかる時間を $\lambda$ とすると，並列プログラムの実行時間は以下の式で見積もられる。
$$\chi\cdot(n\ln\ln n)/p+(\sqrt{n}/\ln\sqrt{n})\lambda\log p$$

## 性能向上
### 偶数を除く
偶数の素数は $2$ のみなので， $3$ 以上の奇数についてのみ考慮する。
プログラムの実行時間はおおよそ
$$\chi(n\ln\ln n)/(2p)+(\sqrt{n}/\ln\sqrt{n})\lambda\log p$$
となる。

### `Broadcast`を排除する
アルゴリズムの3.bで，マークされていない最小の $k$ を探し，他のプロセスに`broadcast`していた。  
プログラムの終了までに`broadcast`は $\sqrt{n}/\ln\sqrt{n}$ 回繰り返される。  
そこで，それぞれのプロセスに自身が担当する $n/p$ 個の整数に加えて， $3,5,7,\cdots,\lfloor\sqrt{n}\rfloor$ の奇数の配列を持たせる。  
$n$ までの合成数は必ず $\lfloor\sqrt{n}\rfloor$ までの素数を因数に持つため，この配列を調べると，マークされていない最小の $k$ を`broadcast`なしで得ることができる。
次式を満たすとき，この修正によって高速化される。
```math
\begin{align*}
&\quad &(\sqrt{n}/\ln\sqrt{n})\lambda\lceil\log p\rceil &> \chi\sqrt{n}\ln\ln\sqrt{n} \\
&\Rightarrow &(\lambda\lceil\log p\rceil)/\ln\sqrt{n} &> \chi\ln\ln\sqrt{n} \\
&\Rightarrow &\lambda &> \chi\ln\ln\sqrt{n}\ln\sqrt{n}/\lceil\log p\rceil
\end{align*}
```
このときの，実行時間の見積もりは
```math
\chi\left((n\ln\ln n)/(2p)+\sqrt{n}\ln\ln\sqrt{n}\right)+\lambda\lceil\log p\rceil
```
ここで，最後の項は合計を求める`reduction`に必要な時間である。

### ループ処理の再編成
マークする巨大な配列の要素は広く分散している。そのため，キャッシュヒット率が低い。  
今，外側のループは $3$ から $\lfloor\sqrt{n}\rfloor$ までの素数について繰り返しており，
内側のループは $3$ から $n$ までのうち各プロセスが担当する部分について繰り返す。  
そこで，内側のループと外側のループを入れ替えると，キャッシュヒット率を上げることができる。
