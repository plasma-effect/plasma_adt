# plasma.ADT
全国で多分5人はいるC++でADT使いたいマン(含plasma)のために作ったADTもどき。
# How To Use
通常版の方は[こっち](https://gist.github.com/plasma-effect/ffc1a66be05eceec1360)で解説してるのでgenericに使える方の解説をします。

lispのリストと同じ型を作ることでplasma.ADT(generic版)の機能に一通り触ることができます(できるはずです多分)  
とりあえずサンプル(中身はmain.cppと同じ)
```cpp
#include<iostream>
#include<list>

#include<plasma_adt/generic_data_type.hpp>

using namespace generic_adt;
using namespace place_holder;

struct list : generic_data_type<list, void, tuple<generic_tag, list>>{};

const auto Nil = list::instance_function<0>();
const auto Tree = list::instance_function<1>();

const auto StdList = pattern_match::generic_recursion<std::list<generic_tag>,list>()
| Nil <= [](auto, auto t) {return std::list<typename decltype(t)::type>{};}
| Tree(0_, 1_) <= [](auto recur, auto, auto v, auto next) {auto lis = recur(next);lis.push_front(v);return lis;};

int main()
{

	const auto IntTree = Tree(type_tag<int>{});
	const auto IntNil = Nil(type_tag<int>{});

	auto v = IntTree(1, IntTree(2, IntNil()));
	
	auto x = StdList(v);
	
	for (auto i : x)
	{
		std::cout << i << std::endl;
	}
}
```
## 準備すべきもの
- C++のコンパイラ
- このリポジトリ(git fetchすると更新があったとき等に楽)
git fetchしたこのリポジトリにパスを通せば使えます。
### (一応)VS2015でのパスの通し方
プロジェクト→  
(プロジェクト名)のプロパティ→  
C/C++→  
追加のインクルードディレクトリ→  
編集→  
目的のフォルダを選択する

## 型を定義しよう
まずどのような型かどうかをgeneric_data_typeに教えてやらないといけません。上のコードではここに当たります。
```cpp
struct list : generic_data_type<list, 
    void,
    tuple<generic_tag, list>>{};
```
generic_data_typeはtemplateクラスです。第1引数にその型の名前を、第2引数以降にその型の構造を代入します。  
型の構造で渡された型のvariantになります。voidだと空クラス、複数の型のtupleならgeneric_adt::tupleを利用します。  
自由に型を変更したいところにgeneric_adt::generic_tagを渡すとそこがgenericになります。
## パターン構成変数を生成しよう
generic_data_typeに渡した型のうち第2変数を第0クラス、以下第1クラス、第2クラス…と呼ぶことにします。  
パターンマッチの構成等に使う変数を生成しないといけません。上のコードでは以下の部分です。
```cpp
const auto Nil = list::instance_function<0>();
const auto Tree = list::instance_function<1>();
```
第nクラスのパターンはinstance_function< n > staticメンバ関数で呼び出せます。  
第0クラス(voidに当たる)をNil  
第1クラス(tuple< generic_tag, list>に当たる)をTreeと名づけます。
## パターンマッチする関数を作ろう
上でリストの構造に関する設定は完了です。次に関数を作る方法を解説します。  
このリストをstd::listに変換する関数を作ります。上のコードでは次に当たります。
```cpp
const auto StdList = pattern_match::generic_recursion<std::list<generic_tag>,list>()
| Nil <= [](auto, auto t) {return std::list<typename decltype(t)::type>{};}
| Tree(0_, 1_) <= [](auto recur, auto, auto v, auto next) {auto lis = recur(next);lis.push_front(v);return lis;};
```
pattern_match::generic_recursionフリー関数でパターンマッチのベースとなるオブジェクトを生成し、
それにoperator|でパターンセットを渡すことでパターンマッチをすることができます。
各パターンセットはパターンと関数のstd::pairになっています。operator<=で一組にできます。  
パターンは渡されたオブジェクトがどのような構造かをチェックするのに使われます。上の例では
- NilのみだとNilから構成されたものを通す
- Tree(0_, 1_)だとTreeであってその第0引数と第1引数をそれぞれ関数の第2引数と第3引数(つまり2個ずれる)
関数には
- 第0引数：再帰用関数
- 第1引数：何のgenericかを表す変数(typename decltype(t)::typeすることでgeneric_tagが何に置き換わっているかわかる)
- 第2引数以降：パターンでoperator"" _が使われているところにあたる変数
が渡されます。ラムダ式を使う場合autoを多様することになります。
## 変数生成用の関数オブジェクトを生成しよう
上のNilとTreeはあくまでパターンであって、ADTのオブジェクトを生成するにはもうひと手間が必要となります。  
パターンのoperator()にplasma_adt::type_tag(型が何かという情報だけが入ったクラスです)の変数を渡すことで
generic_tagの部分がその型に置き換わったオブジェクトを生成できます。
```cpp
const auto IntTree = Tree(type_tag<int>{});
const auto IntNil = Nil(type_tag<int>{});
```
## 変数を生成しよう。
IntTree変数とIntNil変数を使って変数が作れるようになりました。早速使ってみましょう。
```cpp
auto v = IntTree(1, IntTree(2, IntNil()));
```
# 以下上級者向けの話
- サンプルでは継承使ってるけどその型はplace_holderにしか使ってないので実はtypedefでもよい
- 再帰なしバージョンのパターンマッチ関数も作れる。その場合第0引数にtype_tagが渡される
- 変数の型はstd::shared_ptr<...>型
- 構造の問題でクソみたいに長いパラメタヒントを避けられない
