package constantin.renderingx.example;

import java.util.Arrays;

public class Distortion2
{
    public static Distortion2 cardboardV1Distortion() { Distortion2 distortion;
    (distortion = new Distortion2()).coefficients = (float[])CARDBOARD_V1_COEFFICIENTS.clone();
    return distortion; }

    public Distortion2() { this.coefficients = (float[])CARDBOARD_V2_2_COEFFICIENTS.clone(); }


    public Distortion2(Distortion2 paramDistortion) { setCoefficients(paramDistortion.coefficients); }
    public static Distortion2 parseFromProtobuf(float[] paramArrayOffloat) {
        Distortion2 distortion;
        (distortion = new Distortion2()).setCoefficients(paramArrayOffloat);
        return distortion;
    } public float[] toProtobuf() { return (float[])this.coefficients.clone(); }
    public void setCoefficients(float[] paramArrayOffloat) { this.coefficients = (paramArrayOffloat != null) ? (float[])paramArrayOffloat.clone() : new float[0]; }

    public float[] getCoefficients() { return this.coefficients; } public float distortionFactor(float paramFloat) {
        float f1 = 1.0F;
        float f2 = 1.0F;
        float f3 = paramFloat * paramFloat; float[] arrayOfFloat; int i; byte b;
        for (i = (arrayOfFloat = this.coefficients).length, b = 0; b < i; ) { float f = arrayOfFloat[b];
            f2 *= f3;
            f1 += f * f2;
            b++; }
        return f1;
    } public float distort(float paramFloat) { return paramFloat * distortionFactor(paramFloat); }
    public float distortInverse(float paramFloat) { float f1 = paramFloat / 0.9F;
        float f2 = paramFloat * 0.9F;
        float f3 = paramFloat - distort(f1);
        while (Math.abs(f2 - f1) > 1.0E-4D) {
            float f4 = paramFloat - distort(f2);
            float f5 = f2 - f4 * (f2 - f1) / (f4 - f3);
            f1 = f2;
            f2 = f5;
            f3 = f4;
        }
        return f2; }


    private static double[] solveLinear(double[][] a, double[] y) {
        int i = (a[0]).length;
        for (int b = 0; b < i - 1; ) {
            for (int k = b + 1; k < i; ) {
                double d = a[k][b] / a[b][b];
                for (int m = b + 1; m < i; ) {
                    a[k][m] = a[k][m] - d * a[b][m];
                    m++;
                }  y[k] = y[k] - d * y[b];
                k++;
            }  b++;
        }
        double[] x = new double[i];
        for (int j = i - 1; j >= 0; ) {
            double d = y[j];
            for (int k = j + 1; k < i; ) {
                d -= a[j][k] * x[k];
                k++;
            }  x[j] = d / a[j][j];
            j--;
        }
        return x;
    }

    /*private static double[] solveLinear2(double[][] a, double[] y) {
        int n = a[0].length;
        int j;
        double v;
        int i;
        for(int j = 0; j < n - 1; ++j) {
            for(j = j + 1; j < n; ++j) {
                v = a[j][j] / a[j][j];

                for(i = j + 1; i < n; ++i) {
                    a[j][i] -= v * a[j][i];
                }

                y[j] -= v * y[j];
            }
        }
        double[] x = new double[n];
        for(j = n - 1; j >= 0; --j) {
            v = y[j];
            for(i = j + 1; i < n; ++i) {
                v -= a[j][i] * x[i];
            }
            x[j] = v / a[j][j];
        }
        return x;
    }*/


    private static double[] solveLeastSquares(double[][] matA, double[] vecY) {
        int numSamples = matA.length;
        int numCoefficients = matA[0].length;
        double[][] matATA = new double[numCoefficients][numCoefficients];

        int j;
        double sum;
        int i;
        for(int k = 0; k < numCoefficients; ++k) {
            for(j = 0; j < numCoefficients; ++j) {
                sum = 0.0D;

                for(i = 0; i < numSamples; ++i) {
                    sum += matA[i][j] * matA[i][k];
                }

                matATA[j][k] = sum;
            }
        }

        double[] vecATY = new double[numCoefficients];

        for(j = 0; j < numCoefficients; ++j) {
            sum = 0.0D;

            for(i = 0; i < numSamples; ++i) {
                sum += matA[i][j] * vecY[i];
            }

            vecATY[j] = sum;
        }

        return solveLinear(matATA, vecATY);
    }



    public Distortion2 getApproximateInverseDistortion(float maxRadius, int numCoefficients) {
        //int numSamples = true;
        double[][] matA = new double[100][numCoefficients];
        double[] vecY = new double[100];

        for(int i = 0; i < 100; ++i) {
            float r = maxRadius * (float)(i + 1) / 100.0F;
            double rp = (double)this.distort(r);
            double v = rp;

            for(int j = 0; j < numCoefficients; ++j) {
                v *= rp * rp;
                matA[i][j] = v;
            }

            vecY[i] = (double)r - rp;
        }

        double[] vecK = solveLeastSquares(matA, vecY);
        float[] coefficients = new float[vecK.length];

        for(int i = 0; i < vecK.length; ++i) {
            coefficients[i] = (float)vecK[i];
        }

        Distortion2 inverse = new Distortion2();
        inverse.setCoefficients(coefficients);
        return inverse;
    }




    public boolean equals(Object paramObject) {
        if (paramObject == null)
            return false;
        if (paramObject == this)
            return true;
        if (!(paramObject instanceof Distortion2))
            return false;
        Distortion2 distortion = (Distortion2)paramObject;
        return Arrays.equals(this.coefficients, distortion.coefficients);
    }
    public String toString() {
        StringBuilder stringBuilder = (new StringBuilder("D2{\n")).append("  coefficients: [");
        for (byte b = 0; b < this.coefficients.length; ) {
            stringBuilder.append(Float.toString(this.coefficients[b]));
            if (b < this.coefficients.length - 1)
                stringBuilder.append(", ");
            b++;
        }  stringBuilder.append("],\n}");
        return stringBuilder.toString();
    } private static final float[] CARDBOARD_V2_2_COEFFICIENTS = new float[] { 0.34F, 0.55F };
    private static final float[] CARDBOARD_V1_COEFFICIENTS = new float[] { 0.441F, 0.156F };
    private float[] coefficients;



}
