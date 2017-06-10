import au.com.bytecode.opencsv.CSVWriter;
import org.apache.commons.math3.complex.Complex;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class CSVWritterExample {

    public static void exportDataToExcel(CSVWriter csvWriter, Complex[] data) throws FileNotFoundException, IOException {
        String[] values = new String[4096];
        for (int j = 0; j < 4096; j++) {

            double magnitute = Math.log(data[j].abs() + 1);

            values[j] = magnitute + "";
        }
        csvWriter.writeNext(values);

    }
    public static void exportDataToExcel2(CSVWriter csvWriter, ArrayList<String> data) throws FileNotFoundException, IOException {
        String[] values = new String[data.size()];
        for (int i = 0; i < data.size(); i++) {
            values[i] = data.get(i);
        }
        //System.out.println(Arrays.deepToString(values));
        csvWriter.writeNext(values);

    }
}