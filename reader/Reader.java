import com.google.zxing.Binarizer;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.DecodeHintType;
import com.google.zxing.LuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.RGBLuminanceSource;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.common.HybridBinarizer;
import com.google.zxing.qrcode.QRCodeReader;

import javax.imageio.ImageIO;

import java.awt.image.BufferedImage;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;

public class Reader {
  public static void main(String[] args) {
    try {
      String filename = args[0];
      InputStream input = new FileInputStream(filename);
      BufferedImage image = ImageIO.read(input);
      input.close();

      int width = image.getWidth();
      int height = image.getHeight();
      int[] argb = new int[width*height];
      switch (image.getRaster().getSampleModel().getNumBands()) {
        case 1:
          int[] luminosity = image.getRaster().getPixels(0, 0, width, height, (int[]) null);
          for (int i = 0; i < width*height; i++) {
            int l = luminosity[i];
            argb[i] = 0xff000000 | (l << 16) | (l << 8) | l;
          }
          break;
        case 3:
          int[] rgb = image.getRaster().getPixels(0, 0, width, height, (int[]) null);
          for (int i = 0; i < width*height; i++) {
            int r = rgb[3*i + 0];
            int g = rgb[3*i + 1];
            int b = rgb[3*i + 2];
            argb[i] = 0xff000000 | (r << 16) | (g << 8) | b;
          }
          break;
        default:
          throw new RuntimeException(image.getRaster().getSampleModel().getNumBands() + " bands");
      }

      LuminanceSource luminanceSource = new RGBLuminanceSource(width, height, argb);
      Binarizer binarizer = new HybridBinarizer(luminanceSource);
      BinaryBitmap bitmap = new BinaryBitmap(binarizer);

      // dumpBitmap(bitmap);

      Map<DecodeHintType, Object> hints = new HashMap<>();
      hints.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);

      QRCodeReader reader = new QRCodeReader();
      Result result = reader.decode(bitmap, hints);

      byte[] bytes = result.getRawBytes();
      String text = result.getText();
      System.err.println("Decoded " + bytes.length + " bytes");
      for (byte b : bytes) {
        System.err.print(String.format("%02x ", b));
      }
      System.err.println();
      System.err.println("Decoded " + text.length() + " chars");
      for (int i = 0; i < text.length(); i++) {
        System.err.print(String.format("%02x ", Character.codePointAt(text, i)));
      }
      System.err.println();
    } catch (Exception ex) {
      ex.printStackTrace();
      System.exit(1);
    }
  }

  private static void dumpBitmap(BinaryBitmap bitmap) throws Exception {
    PrintWriter out = new PrintWriter("bitmap.ppm", "ASCII");

    BitMatrix matrix = bitmap.getBlackMatrix();
    out.println("P3 " + matrix.getWidth() + " " + matrix.getHeight() + " 255");
    for (int i = 0; i < matrix.getHeight(); i++) {
      for (int j = 0; j < matrix.getWidth(); j++) {
        out.println(matrix.get(j, i) ? "0 0 0" : "255 255 255");
      }
    }

    out.close();
  }
}
