package stingyflink;

import org.apache.flink.streaming.api.datastream.DataStream;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.api.java.operators.DataSource;
import org.apache.flink.util.Collector;
import org.apache.flink.api.java.tuple.Tuple2;
import org.apache.flink.streaming.api.TimeCharacteristic;
import org.apache.flink.api.common.typeinfo.Types;
import org.apache.flink.api.java.utils.ParameterTool;

public class StingySketchJob {
    public static void main(String[] args) throws Exception {
        final ParameterTool params = ParameterTool.fromArgs(args);

        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        env.setStreamTimeCharacteristic(TimeCharacteristic.EventTime);

        // DataStream<Integer> source = env.socketTextStream("127.0.0.1", 9000, "\n");
        DataStream<Tuple2<Integer, Long>> source = env
		.readTextFile(params.get("input"))
		.map(x -> {
		   String[] value = x.split(",");
		   return new Tuple2<Integer, Long>((int)Long.parseLong(value[0]), Long.parseLong(value[1])); })
		.returns(Types.TUPLE(Types.INT, Types.LONG));
        
        DataStream<SketchElement> query = source
            .keyBy(value -> value.f0)
            .process(new StingySketchProcess())
            .name("stingy-sketch-query");

        query.print().setParallelism(1);

        env.execute("Stingy Sketch Query");
    }
}
