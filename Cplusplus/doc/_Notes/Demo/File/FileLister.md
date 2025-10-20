### 功能说明

1. **基本功能**：
   - 提取指定文件夹下的文件名
   - 支持递归子文件夹（可控制深度）
   - 支持文件扩展名过滤（大小写不敏感）
   - 支持目录名过滤
   - 可自定义输出分隔符
   - 可选择换行/不换行输出
2. **参数说明**：
   - `--input`：要扫描的文件夹路径（必需）
   - `--output`：输出文件路径（默认输出到控制台）
   - `--recursive`：递归深度（0=当前目录，1=1级子目录，-1=无限递归）
   - `--pattern`：文件扩展名模式（如 `lib`, `*.txt`）
   - `--dir-name`：递归时匹配的目录名模式
   - `--no-newline`：禁用换行输出
   - `--add-end`：最后一行也添加分隔符
   - `--separator`：自定义分隔符（换行模式默认无换行分隔符，不换行模式默认使用分号 `;`）

**使用示例**：

```bash
# 基本用法：提取当前目录所有文件（换行输出）
filelister.exe --input .

# 提取DLL文件（不换行，分号分隔）
filelister.exe --input C:\Windows\System32 --pattern *.dll --no-newline

# 递归提取（3级深度，输出到文件）
filelister.exe --input src --recursive 3 --pattern *.h --output headers.txt

# 自定义分隔符和目录过滤
filelister.exe --input projects --recursive -1 --dir-name libs --pattern *.lib --separator "|"

FileLister.exe --input . --pattern *.lib --separator ";" --no-newline --output LISTn.TXT
FileLister.exe --input . --pattern lib --no-newline --separator "|" --output LISTn.TXT
```