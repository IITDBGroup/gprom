package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMProjectionOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/**
	 * projection expressions<br>
	 * C type : GProMList*
	 */
	public org.gprom.jdbc.jna.GProMList.ByReference projExprs;
	public GProMProjectionOperator() {
		super();
	}
	public GProMProjectionOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "projExprs");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param projExprs projection expressions<br>
	 * C type : GProMList*
	 */
	public GProMProjectionOperator(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference projExprs) {
		super();
		this.op = op;
		this.projExprs = projExprs;
		write();
	}
	public static class ByReference extends GProMProjectionOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMProjectionOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference projExprs){
			super(op,projExprs);
		}
		
	};
}
