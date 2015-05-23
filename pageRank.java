package org.myorg;

        import java.io.IOException;
        import java.util.*;

        import org.apache.hadoop.fs.Path;
        import org.apache.hadoop.conf.*;
        import org.apache.hadoop.io.*;
        import org.apache.hadoop.mapred.*;
        import org.apache.hadoop.util.*;
public class pageRank {
        public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, IntWritable> {
        private final static IntWritable one = new IntWritable(1);
        private Text word = new Text();

        public void map(LongWritable key, Text value, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {
          String line = value.toString();
          StringTokenizer tokenizer = new StringTokenizer(line);
          while (tokenizer.hasMoreTokens()) {
            word.set(tokenizer.nextToken());
            output.collect(word, one);
          }
        }
      }
        public static class Map2 extends MapReduceBase implements Mapper<LongWritable, Text, Text, IntWritable> {
        private final static IntWritable one = new IntWritable(1);
        private Text word = new Text();

        public void map(LongWritable key, Text value, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {
          String line = value.toString();
          StringTokenizer tokenizer = new StringTokenizer(line);
          while (tokenizer.hasMoreTokens()) {
            word.set(tokenizer.nextToken());
            output.collect(word, one);
          }
        }
      }

      public static class Reduce extends MapReduceBase implements Reducer<Text, IntWritable, Text, IntWritable> {
        public void reduce(Text key, Iterator<IntWritable> values, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {
          int sum = 0;
          while (values.hasNext()) {
            sum += values.next().get();
          }
          output.collect(key, new IntWritable(sum));
        }
      }
      public static class Reduce2 extends MapReduceBase implements Reducer<Text, IntWritable, Text, IntWritable> {
          public void reduce(Text key, Iterator<IntWritable> values, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {
            int sum = 0;
            while (values.hasNext()) {
              sum += values.next().get();
            }
            output.collect(key, new IntWritable(sum));
          }
        }
      public static void main(String[] args) throws Exception {
          JobConf conf = new JobConf(pageRank.class);
          conf.setJobName("wordcount");
          conf.setOutputKeyClass(Text.class);
          conf.setOutputValueClass(IntWritable.class);

          conf.setMapperClass(Map.class);
          conf.setCombinerClass(Reduce.class);
          conf.setReducerClass(Reduce.class);

          conf.setInputFormat(TextInputFormat.class);
          conf.setOutputFormat(TextOutputFormat.class);

          FileInputFormat.setInputPaths(conf, new Path(args[0]));
          FileOutputFormat.setOutputPath(conf, new Path(args[1]));

          JobClient.runJob(conf);

          JobConf conf2 = new JobConf(pageRank.class);
          conf2.setJobName("wordcount");
          conf2.setOutputKeyClass(Text.class);
          conf2.setOutputValueClass(IntWritable.class);

          conf2.setMapperClass(Map2.class);
          conf2.setCombinerClass(Reduce2.class);
          conf2.setReducerClass(Reduce2.class);

          conf2.setInputFormat(TextInputFormat.class);
          conf2.setOutputFormat(TextOutputFormat.class);

          FileInputFormat.setInputPaths(conf2, new Path(args[0]));
          FileOutputFormat.setOutputPath(conf2, new Path(args[1]));

          JobClient.runJob(conf2);
	}
}