# CN2020-Homework3 - Simple Video Streaming System
## Usage
### Compile
產生 agent 、 sender 與 receiver 執行檔。
```shell
$ make
```

### Execute
分別在三個 terminal 開啟 sender 、 receiver 與 agent 。
```shell
$ ./sender <sender port> <agent IP> <agent port>
$ ./receiver <recv port> <agent IP> <agent port>
$ ./agent <sender IP> <recv IP> <sender port> <agent port> <recv port> <loss rate>
```
或是使用 `exec.sh` 來執行（不用手動輸入參數）。
```shell
$ ./exec.sh sender
# same as ./sender 7001 local 7002
$ ./exec.sh receiver
# same as ./receiver 7003 local 7002
$ ./exec.sh agent <loss rate>
# same as ./agent local local 7001 7002 7003 <loss rate>
```

## Functions
- 先依任意順序執行 sender 、receiver 、agent ，然後將影片放入 sender_dir 中。接下來在 receiver 內輸入 `play <video name>`。
    - 未做確認影片的檢測，因此請特別注意。
- 沒有使用 ffmpeg 。