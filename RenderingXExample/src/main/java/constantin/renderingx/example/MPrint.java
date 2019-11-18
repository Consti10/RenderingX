package constantin.renderingx.example;

import java.nio.ByteBuffer;

public class MPrint {
    public static void mprint1(double[] p1){
        StringBuilder s=new StringBuilder();
        s.append("[");
        for(int i=0;i<p1.length;i++){
            s.append(p1[i]).append(",");
        }
        s.append("[");
        System.out.println("p1:"+s.toString());
    }

    public static void mprint2(double[][] p2){
        StringBuilder s=new StringBuilder();
        System.out.println("p2:[\n");
        s.append("[");
        for(int i=0;i<p2.length;i++){
            for(int j=0;j<p2[i].length;j++){
                s.append((float)p2[i][j]).append(",");
            }
            System.out.println(s.toString());
            s=new StringBuilder();
        }
        System.out.println("]");
    }

    public static void mprint3(double[][] p2){
        StringBuilder s=new StringBuilder();
        System.out.println("p2:[\n");
        s.append("[");
        for(int i=0;i<p2.length;i++){
            for(int j=0;j<p2[i].length;j++){
                //long longBits = Double.doubleToLongBits(p2[i][j]);
                //String binary = Long.toBinaryString(longBits);
                //s.append(asByteString(p2[i][j])).append(",");
                String bin=Long.toBinaryString(Double.doubleToRawLongBits(p2[i][j]));
                s.append(bin).append(",");
            }
            System.out.println(s.toString());
            s=new StringBuilder();
        }
        System.out.println("]");
    }

    public static byte[] toByteArray(double value) {
        byte[] bytes = new byte[8];
        ByteBuffer.wrap(bytes).putDouble(value);
        return bytes;
    }




}
