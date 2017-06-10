import au.com.bytecode.opencsv.CSVWriter;
import org.apache.commons.math3.complex.Complex;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.UnsupportedAudioFileException;
import java.io.*;
import java.math.RoundingMode;
import java.net.UnknownHostException;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedHashMap;

public class FingerprintSample implements Runnable{

    private static final int MIN_MAP_SIZE = 108;//accoding to the chunk size we use for matching this can be changed 10s
    private static final int MAX_MAP_SIZE = 968;//accoding to the chunk size we use for matching this can be changed 90s
    static double meanCount = 0, peak = 0, base=1000;
    private static LinkedHashMap<String, LinkedHashMap<Double, Long>> databaseHashesMap = new LinkedHashMap<String, LinkedHashMap<Double, Long>>();
    private static CSVWriter csvWriter;
    private static int count = 0, count2 = 0;
    private final int[] RANGE = new int[]{40, 120, 300};
    private final int[] RANGE2 = new int[]{3980, 4040, 4096};
    PrintWriter out1 = new PrintWriter("filename.txt");
    private long FUZ_FACTOR = 2;
    private int LOWER_LIMIT = 40;
    private int UPPER_LIMIT = 300;
    private int LOWER_LIMIT2 = 3980;
    private int UPPER_LIMIT2 = 4096;
    private static ArrayList<Double> shift = new ArrayList<Double>();
    private static int processedCount=0;
    private static FingerprintSample fs;

    public FingerprintSample() throws FileNotFoundException{
    }
    @Override
    public void run() {
        while(true) {
            int newCount=0;
            try {
                BufferedReader inp = new BufferedReader(
                        new FileReader("C:/Users/Sampath/Desktop/Soft/IterativeInputRead.csv"));
                while (inp.readLine() != null) {
                    ++newCount;
                }

                if(newCount>processedCount){
                    BufferedReader input = new BufferedReader(
                            new FileReader("C:/Users/Sampath/Desktop/Soft/IterativeInputRead.csv"));
                    String str;
                    int cnt=0;
                    while ((str = input.readLine()) != null ) {
                        ++cnt;
                        if(cnt>processedCount) {
                            String[] arr = str.split(",");
                            String name = arr[0];
                            String timeShift = arr[1];
//                            System.out.println(name);
                            fs.matchingAd("C:/Users/Sampath/Desktop/Soft/video/recording.wav",name,timeShift);
                            ++processedCount;
                        }
                    }
                }
                if(processedCount==newCount){
                    System.out.println("...");
                }

                Thread.sleep(1000);

            }catch (IOException ee) {
                ee.printStackTrace();
                System.out.println("input file read error");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
    public static void main(String[] args) throws IOException, UnsupportedAudioFileException {
        /*File file = new File("D:/AnchorPediaPro1.csv");
        if (!file.isFile()) {
            file.createNewFile();
        }*/

        fs=new FingerprintSample();
        fs.readDB();

        Thread t = new Thread(new FingerprintSample());
        t.start();
    }

    public void readDB(){
        try {
            BufferedReader in = new BufferedReader(
                    new FileReader("audio/timehashMapNew.txt"));
            String str;
            while ((str = in.readLine()) != null) {
                String[] ar = str.split(",");
                LinkedHashMap<Double, Long> lhm = new LinkedHashMap<Double, Long>();
                lhm.put(new Double(ar[1]), new Long(ar[2]));
                if (databaseHashesMap.containsKey(ar[0]))
                    databaseHashesMap.get(ar[0]).put(new Double(ar[1]), new Long(ar[2]));
                else
                    databaseHashesMap.putIfAbsent(ar[0], lhm);
            }
            in.close();
            

        } catch (IOException e) {
            System.out.println("Stream Read Error");
        }

 }

 public void matchingAd(String path,String name,String timeShift){

     //Loading stream and matching
     File fileIn = new File(path);
     LinkedHashMap<Double, Long> audioMap = null;

     try {
         audioMap = fs.generateFingerprint(fileIn,name,timeShift);
     } catch (UnsupportedAudioFileException e) {
         e.printStackTrace();
     } catch (IOException e) {
         e.printStackTrace();
     }

       /* BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream("audio/Astra3072.csv"), "utf-8"));
        for (int i = 0; i < shift.size()-1; i++) {
            writer.write(shift.get(i)+"\n");
        }
        writer.write(shift.get(shift.size()-1).toString());
        writer.close();*/

     //CSVWritterExample.exportDataToExcel2(csvWriter,shift);

     //matching
     /**
      * @Desc Matching
      */
      Matching m=new Matching();

     //parsing chunk block to match
     try {
         m.slidingWindowMatchNew(audioMap,databaseHashesMap,name,timeShift);
     } catch (UnknownHostException e) {
         e.printStackTrace();
     }

     if (MIN_MAP_SIZE <= audioMap.size() && MAX_MAP_SIZE >= audioMap.size()) {
         //Matching match=new Matching();
     }

//     try {
//         System.out.println("Mean magnitude value of the signal : "+new FingerprintSample().roundDoubles(meanCount/(count*count2)));
//         System.out.println("Maximum magnitude value of the signal : "+new FingerprintSample().roundDoubles(peak));
//         System.out.println("Minimum magnitude value of the signal : "+new FingerprintSample().roundDoubles(base));
//     } catch (FileNotFoundException e) {
//         e.printStackTrace();
//     }
 }

    public LinkedHashMap<Double, Long> generateFingerprint(File sampleFile,String name,String timeShift) throws UnsupportedAudioFileException, IOException {
        double duration=0;
        if (databaseHashesMap.containsKey(name)) {
            duration = databaseHashesMap.get(name).size() * 4096 / 44100;
            System.out.println("Duration : "+duration);
        }
        else
            System.out.println("DB doesnt hv the advertisement");


        AudioInputStream in = AudioSystem.getAudioInputStream(sampleFile);
        AudioFormat baseFormat = in.getFormat();
        String[] strar=timeShift.split("-");

        int sstime=(Integer.parseInt(strar[0]));
        long dd=in.skip((long) (sstime*(baseFormat.getFrameSize() * (int)baseFormat.getFrameRate())));
        //System.out.println(dd+" "+187331*sstime);

        //System.out.println(baseFormat.getFrameSize());
        AudioFormat decodedFormat = new AudioFormat(AudioFormat.Encoding.PCM_UNSIGNED,
                baseFormat.getSampleRate(),
                16,
                baseFormat.getChannels(),
                baseFormat.getChannels() * 2,
                baseFormat.getSampleRate(),
                false);
        int samplesPerChannel = (int) Math.rint(baseFormat.getFrameRate());
        int channelCount = baseFormat.getChannels();
        int size = samplesPerChannel * channelCount * 2;
        AudioInputStream din = AudioSystem.getAudioInputStream(decodedFormat, in);
        byte[] buffer = new byte[size];
        double[][] channels = new double[samplesPerChannel][];
        double[] avgBuffer = new double[samplesPerChannel];
        LinkedHashMap<Double, Long> hashTimeMap = new LinkedHashMap<Double, Long>();
        double second = 0;
        double timeIncrement;
        float sampleRate = baseFormat.getSampleRate();
        int chunkSize;

        for (int i = 0; i < channelCount; i++) {
            channels[i] = new double[samplesPerChannel];
        }


        double time = 0;

        //BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream("audio/SkipTesting.csv"), "utf-8"));
        while (din.read(buffer, 0, size) > -1 && duration>time) {

            //System.out.println(Arrays.toString(buffer));
            //count++;
            LinkedHashMap<Double, Long> streamLHM = new LinkedHashMap<Double, Long>();
           // System.out.println(Arrays.toString(buffer));


//            for (int i = 0; i < shift.size()-1; i++) {
//                writer.write(shift.get(i)+"\n");
//            }
//            writer.write(shift.get(shift.size()-1).toString());
//            writer.close();

            for (int ch = 0; ch < channelCount; ch++) {
                for (int i = 0; i < samplesPerChannel; i++) {
                    channels[ch][i] = (buffer[channelCount * 2 * i + 2 * ch] + 256.0 * buffer[channelCount * 2 * i + 2 * ch + 1]) / 32768.0;
                }
            }
            for (int i = 0; i < samplesPerChannel; i++) {
                double temp = channels[0][i];
                for (int ch = 1; ch < channelCount; ch++) {
                    temp += channels[ch][i];
                }
                avgBuffer[i] = temp / channelCount;
            }

            int totalSize = avgBuffer.length;
            if (32000 <= sampleRate) {
                chunkSize = 4096;
                timeIncrement = 1 / ((double) (sampleRate / chunkSize));
            } else if ((sampleRate <= 32000) && (sampleRate > 16000)) {
                chunkSize = 2048;
                timeIncrement = 1 / ((double) (sampleRate / chunkSize));
            } else if ((sampleRate <= 16000) && (sampleRate > 8000)) {
                chunkSize = 1024;
                timeIncrement = 1 / ((double) (sampleRate / chunkSize));
            } else {
                chunkSize = 512;
                timeIncrement = 1 / ((double) (sampleRate / chunkSize));
            }


            double[] finalBuffer = new double[chunkSize];
            int chunkCount = totalSize / chunkSize;
            double[][] arrayWithChunks = new double[chunkCount][chunkSize];
            double[][] highscores = new double[chunkCount][4];
            double[][] points = new double[chunkCount][4];

            for (int i = 0; i < chunkCount; i++) {
                for (int j = 0; j < 4; j++) {
                    highscores[i][j] = 0;
                }
            }

            for (int i = 0; i < chunkCount; i++) {
                for (int j = 0; j < 4; j++) {
                    points[i][j] = 0;
                }
            }
            /*int adj;
            if(count==0){
                adj=3584;
            }else{
                adj=0;
            }*/
            for (int i = 0; i < chunkCount; i++) {
                double[] oneChunk = new double[chunkSize];
                for (int chunks = 0; chunks < chunkSize; chunks++) {
                    oneChunk[chunks] = avgBuffer[(i * chunkSize) + chunks];
                }

                arrayWithChunks[i] = oneChunk;
            }
            count++;

            for (int arrayWithChunksIndex = 0; arrayWithChunksIndex < arrayWithChunks.length; arrayWithChunksIndex++) {
                count2++;
                System.arraycopy(arrayWithChunks[arrayWithChunksIndex], 0, finalBuffer, 0, arrayWithChunks[arrayWithChunksIndex].length);
                Complex[] complex = new Complex[chunkSize];
                Complex[] fftComplex;
                for (int i = 0; i < finalBuffer.length; i++) {
                    //Put the time domain data into a complex number with imaginary part as 0:
                    complex[i] = new Complex(finalBuffer[i], 0);
                }
                fftComplex = FFT.fft(complex);
//                System.out.println(Arrays.toString(fftComplex));
                //CSVWritterExample.exportDataToExcel(csvWriter,fftComplex);



//                //Finding mean of the full signal
//                meanCount+=MeanCalculation.meanOfCommercial(fftComplex,chunkSize);
//
//                //Finding max of the full signal
//                double maxValueofChunk=MaxCalculation.maxOfCommercial(fftComplex,chunkSize);
//                if(peak < maxValueofChunk){
//                    peak = maxValueofChunk;
//                }
//
//                //Finding min of the full signal
//                double minValueofChunk=MinCalculation.minOfCommercial(fftComplex,chunkSize);
//                if(base > minValueofChunk){
//                    base = minValueofChunk;
//                }


                double[] getPoints = getTopFreqValuesArray(fftComplex, chunkSize);
                double[] getPoints2 = getTopFreqValuesArray2(fftComplex, chunkSize);
                long hash = hash((long) getPoints[0], (long) getPoints[1], (long) getPoints2[0], (long) getPoints2[1]);
                //1
                second = roundDoubles(second + timeIncrement);
                //2
                hashTimeMap.put(second, hash);
                //shift.add((double) hash);
               

            }
            ++time;

        }
        in.close();
        return hashTimeMap;
    }

    private double roundDoubles(Double valueToConvert) {
        DecimalFormat decimalFormat = new DecimalFormat("#.##");
        decimalFormat.setRoundingMode(RoundingMode.HALF_UP);
        Double d = valueToConvert;
        double valueConverted = Double.parseDouble(decimalFormat.format(d));
        return valueConverted;
    }


    private double[] getTopFreqValuesArray1(double[] chunkArray) {
        double[] points = new double[4];
        for (int i = 0; i + 1 < RANGE.length; i++) {
            double mag = 0;
            for (int j = RANGE[i]; j < RANGE[i + 1]; j++) {
                double temp = chunkArray[j];
                if (temp > mag) {
                    mag = temp;
                }
            }
            points[i] = roundDoubles(mag);
        }
        return points;
    }

    public int getIndex(int freq) {
        int i = 0;
        while (RANGE[i] <= freq) {
            i++;
        }
        return i;
    }


    public double[] getTopFreqValuesArray(Complex[] chunkComplexArray, int chunkCount) {
        double[] highscores = new double[2];
        double[] points = new double[2];
        //System.out.println("complex : "+chunkComplexArray[LOWER_LIMIT].abs()+"      "+chunkComplexArray[UPPER_LIMIT].abs());
        for (int freq = LOWER_LIMIT; freq < UPPER_LIMIT; freq++) {
            double magnitute = Math.log(chunkComplexArray[freq].abs() + 1);
            int index = getIndex(freq);
            if (magnitute > highscores[index - 1]) {
                highscores[index - 1] = magnitute;
                points[index - 1] = freq;
            }

        }
        return points;
    }

    public int getIndex2(int freq) {
        int i = 0;
        while (RANGE2[i] <= freq) {
            i++;
        }
        return i;
    }


    public double[] getTopFreqValuesArray2(Complex[] chunkComplexArray, int chunkCount) {
        double[] highscores = new double[2];
        double[] points = new double[2];
        //System.out.println("complex : "+chunkComplexArray[LOWER_LIMIT].abs()+"      "+chunkComplexArray[UPPER_LIMIT].abs());
        for (int freq = LOWER_LIMIT2; freq < UPPER_LIMIT2; freq++) {
            double magnitute = Math.log(chunkComplexArray[freq].abs() + 1);
            int index = getIndex2(freq);
            if (magnitute > highscores[index - 1]) {
                highscores[index - 1] = magnitute;
                points[index - 1] = freq;
            }

        }
        return points;
    }

    //Using a little bit of error-correction, damping
    long hash(long p1, long p2, long p3, long p4) {
        //System.out.println(p1+"--"+p2+"--"+p3+"--"+p4);
        return (p4 - (p4 % FUZ_FACTOR)) * 1000000000 + (p3 - (p3 % FUZ_FACTOR))
                * 1000000 + (p2 - (p2 % FUZ_FACTOR)) * 1000 + (p1 - (p1 % FUZ_FACTOR));
    }


}