// 导出每个函数的特征，供跨版本函数匹配（scripts/fl_match.py）使用。
// 输出 JSONL：每行一个函数。
//   e: 入口（相对镜像基址），n: 字节数，ic: 指令数，m: 助记符序列哈希，
//   mn: 助记符序列（前 64 条，用于相似度），c: 调用的函数入口列表（按出现顺序），
//   s: 引用的字符串，k: 非地址常量（立即数），d: 引用的数据地址（相对基址，非字符串）
// 用法：analyzeHeadless <proj_dir> <proj> -process libg.so -noanalysis -readOnly
//        -scriptPath re/ghidra_scripts -postScript ExportFuncFeatures.java <out.jsonl>
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.data.StringDataInstance;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class ExportFuncFeatures extends GhidraScript {
    private static String esc(String s) {
        StringBuilder b = new StringBuilder();
        for (char ch : s.toCharArray()) {
            if (ch == '"' || ch == '\\') b.append('\\').append(ch);
            else if (ch < 0x20 || ch > 0x7e) b.append(String.format("\\u%04x", (int) ch));
            else b.append(ch);
        }
        return b.toString();
    }

    @Override
    protected void run() throws Exception {
        String out = getScriptArgs()[0];
        long base = currentProgram.getImageBase().getOffset();
        Listing listing = currentProgram.getListing();
        FunctionManager fm = currentProgram.getFunctionManager();
        try (Writer w = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(out), StandardCharsets.UTF_8))) {
            for (Function f : fm.getFunctions(true)) {
                if (monitor.isCancelled()) break;
                if (f.isExternal() || f.isThunk()) continue;
                long entry = f.getEntryPoint().getOffset() - base;
                List<Long> calls = new ArrayList<>();
                List<String> strs = new ArrayList<>();
                List<Long> consts = new ArrayList<>();
                List<Long> datas = new ArrayList<>();
                StringBuilder mn = new StringBuilder();
                int ic = 0;
                InstructionIterator it = listing.getInstructions(f.getBody(), true);
                List<String> mnList = new ArrayList<>();
                while (it.hasNext()) {
                    Instruction ins = it.next();
                    ic++;
                    String m = ins.getMnemonicString();
                    mn.append(m).append(';');
                    if (mnList.size() < 64) mnList.add(m);
                    for (Reference r : ins.getReferencesFrom()) {
                        Address to = r.getToAddress();
                        if (r.getReferenceType().isCall()) {
                            Function cf = fm.getFunctionAt(to);
                            if (cf != null && cf.isThunk()) {
                                Function tf = cf.getThunkedFunction(true);
                                if (tf != null) cf = tf;
                            }
                            if (cf != null && cf.isExternal()) {
                                strs.add("@" + cf.getName());
                            } else {
                                calls.add(to.getOffset() - base);
                            }
                        } else if (r.getReferenceType().isData() && to.isMemoryAddress()) {
                            Data d = listing.getDataAt(to);
                            if (d != null && d.hasStringValue()) {
                                Object v = d.getValue();
                                if (v != null) strs.add(v.toString());
                            } else {
                                datas.add(to.getOffset() - base);
                            }
                        }
                    }
                    for (int i = 0; i < ins.getNumOperands(); i++) {
                        for (Object o : ins.getOpObjects(i)) {
                            if (o instanceof Scalar) {
                                long v = ((Scalar) o).getSignedValue();
                                if (Math.abs(v) >= 0x10 && Math.abs(v) < 0x100000) consts.add(v);
                            }
                        }
                    }
                }
                StringBuilder b = new StringBuilder();
                b.append("{\"e\":").append(entry)
                 .append(",\"n\":").append(f.getBody().getNumAddresses())
                 .append(",\"ic\":").append(ic)
                 .append(",\"m\":").append(mn.toString().hashCode())
                 .append(",\"mn\":\"").append(esc(String.join(" ", mnList))).append('"')
                 .append(",\"c\":").append(calls.toString())
                 .append(",\"d\":").append(datas.toString())
                 .append(",\"k\":").append(consts.toString())
                 .append(",\"s\":[");
                for (int i = 0; i < strs.size(); i++) {
                    if (i > 0) b.append(',');
                    b.append('"').append(esc(strs.get(i))).append('"');
                }
                b.append("]}\n");
                w.write(b.toString());
            }
        }
        println("exported to " + out);
    }
}
