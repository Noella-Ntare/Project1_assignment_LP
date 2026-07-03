# Week 3 Formative Assignment — How to Compile & Run

---

## Project 1 — Suspicious Binary

```bash
gcc -o data_sync data_sync.c

file data_sync
objdump -f data_sync
objdump -h data_sync
nm data_sync
nm -u data_sync
strings data_sync
```

---

## Project 2 — System Call Monitoring

```bash
gcc -o backup_tool backup_tool.c

strace -o strace_output.txt -e trace=file,process,read,write ./backup_tool

cat strace_output.txt
```

---

## Project 3 — Python C Extension

```bash
python3 setup.py build_ext --inplace

python3 stats_python.py

python3 benchmark.py
```

---

## Project 4 — Signal-Based Server

**Terminal 1:**
```bash
gcc -Wall -o monitor_service monitor_service.c

./monitor_service
```

**Terminal 2** (replace `<pid>` with the number printed on startup):
```bash
kill -SIGUSR1 <pid>
kill -SIGINT <pid>
kill -SIGTERM <pid>
```
