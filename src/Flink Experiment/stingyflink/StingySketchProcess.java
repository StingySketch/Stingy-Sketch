package stingyflink;

import org.apache.flink.api.common.state.ValueState;
import org.apache.flink.api.common.state.ValueStateDescriptor;
import org.apache.flink.api.common.typeinfo.Types;
import org.apache.flink.configuration.Configuration;
import org.apache.flink.streaming.api.functions.KeyedProcessFunction;
import org.apache.flink.util.Collector;
import org.apache.flink.api.java.tuple.Tuple2;

public class StingySketchProcess extends KeyedProcessFunction<Integer, Tuple2<Integer, Long>, SketchElement> {
    public StingySketchStruct sketch=new StingySketchStruct();

    @Override
    public void processElement (
      Tuple2<Integer, Long> item,
      Context ctx,
      Collector<SketchElement> collector) throws Exception {
        sketch.insert(item.f0);
        int que=sketch.query(item.f0);
        SketchElement re = new SketchElement(item.f0,que);
        collector.collect(re);
    }
}