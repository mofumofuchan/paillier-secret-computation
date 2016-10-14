# Secure multi-party computation of sum of vector
## 概略
- 各クライアントが暗号化した整数値のベクタを送信
- その和が計算用ノードで暗号化したまま計算
- サーバー側で和（統計量）のみを復号化して表示

## 特記事項
- 現状では，ベクトルの次元を#define VECTOR_DEMENSION <次元数>で決め打ち．
- client.cpp, server.cpp, compute.cppの各々のファイルで，VECTOR_DEMENSIONは定義されている．

## example
```
	./server -d tmp/ -k tmp/ -p 111111 -l 100
	./compute -d tmp/ -k tmp/ -m localhost -p 111111 -q 22222 -i 100 -l sample_user_list -o encresult

	./client -d tmp/ -k tmp/ -m localhost -p 111111 -c localhost -q 22222 -i 1 -- 10 20 30
   ./client -d tmp/ -k tmp/ -m localhost -p 111111 -c localhost -q 22222 -i 2 -- 20 20 20
   ./client -d tmp/ -k tmp/ -m localhost -p 111111 -c localhost -q 22222 -i 3 -- -10 -20 30

(server) result from compute server: 20, 20, 80, 
```
