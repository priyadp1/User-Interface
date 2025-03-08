# 🐚 osh - A Custom UNIX Shell in C

### Author: Prisha  
### Technologies: C, POSIX, `execvp()`, `pipe()`, `dup2()`, `termios`, `waitpid()`

## 📌 Overview  
**osh** (short for **Prisha's Shell**) is a custom UNIX shell written in **C** that supports:  
- **Command execution (`ls`, `grep`, etc.)**  
- **Pipes (`|`) for inter-process communication**  
- **Input (`<`) and output (`>`) redirection**  
- **Background execution (`&`)**  
- **Arrow key history navigation (`↑`, `↓`)**  
- **Raw mode terminal control (`termios`)**  

## 🚀 Installation & Usage  
### **1️⃣ Clone the Repository**  
```bash
git clone https://github.com/priyadp1/osh.git
cd osh
### **2️⃣ Build the Shell
make
3️⃣ Run the Shell
bash
Copy
Edit
./osh
