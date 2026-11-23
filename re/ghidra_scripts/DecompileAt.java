// 反编译指定地址（相对镜像基址）所在的函数，输出到文件；地址处没有函数时先创建。
// 用法：... -postScript DecompileAt.java <out.c> <addr1> <addr2> ...
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

import java.io.*;
import java.nio.charset.StandardCharsets;

public class DecompileAt extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        long base = currentProgram.getImageBase().getOffset();
        try (Writer w = new OutputStreamWriter(new FileOutputStream(args[0]), StandardCharsets.UTF_8)) {
            for (int i = 1; i < args.length; i++) {
                long off = Long.decode(args[i]);
                Address a = toAddr(base + off);
                Function f = getFunctionContaining(a);
                if (f == null) {
                    disassemble(a);
                    f = createFunction(a, null);
                }
                w.write("// ===== " + args[i] + " =====\n");
                if (f == null) {
                    w.write("// no function\n\n");
                    continue;
                }
                w.write("// entry " + Long.toHexString(f.getEntryPoint().getOffset() - base) + "\n");
                DecompileResults r = di.decompileFunction(f, 120, monitor);
                w.write(r.decompileCompleted() ? r.getDecompiledFunction().getC() : "// decompile failed\n");
                w.write("\n");
            }
        }
    }
}
