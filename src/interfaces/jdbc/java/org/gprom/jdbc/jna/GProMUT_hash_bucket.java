package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMUT_hash_bucket extends GProMStructure {
	/** C type : GProMUT_hash_handle* */
	public org.gprom.jdbc.jna.GProMUT_hash_handle.ByReference hh_head;
	public int count;
	/**
	 * expand_mult is normally set to 0. In this situation, the max chain length<br>
	 * threshold is enforced at its default value, HASH_BKT_CAPACITY_THRESH. (If<br>
	 * the bucket's chain exceeds this length, bucket expansion is triggered). <br>
	 * However, setting expand_mult to a non-zero value delays bucket expansion<br>
	 * (that would be triggered by additions to this particular bucket)<br>
	 * until its chain length reaches a *multiple* of HASH_BKT_CAPACITY_THRESH.<br>
	 * (The multiplier is simply expand_mult+1). The whole idea of this<br>
	 * multiplier is to reduce bucket expansions, since they are expensive, in<br>
	 * situations where we know that a particular bucket tends to be overused.<br>
	 * It is better to let its chain length grow to a longer yet-still-bounded<br>
	 * value, than to do an O(n) bucket expansion too often.
	 */
	public int expand_mult;
	public GProMUT_hash_bucket() {
		super();
	}
	public GProMUT_hash_bucket(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("hh_head", "count", "expand_mult");
	}
	/**
	 * @param hh_head C type : GProMUT_hash_handle*<br>
	 * @param expand_mult expand_mult is normally set to 0. In this situation, the max chain length<br>
	 * threshold is enforced at its default value, HASH_BKT_CAPACITY_THRESH. (If<br>
	 * the bucket's chain exceeds this length, bucket expansion is triggered). <br>
	 * However, setting expand_mult to a non-zero value delays bucket expansion<br>
	 * (that would be triggered by additions to this particular bucket)<br>
	 * until its chain length reaches a *multiple* of HASH_BKT_CAPACITY_THRESH.<br>
	 * (The multiplier is simply expand_mult+1). The whole idea of this<br>
	 * multiplier is to reduce bucket expansions, since they are expensive, in<br>
	 * situations where we know that a particular bucket tends to be overused.<br>
	 * It is better to let its chain length grow to a longer yet-still-bounded<br>
	 * value, than to do an O(n) bucket expansion too often.
	 */
	public GProMUT_hash_bucket(org.gprom.jdbc.jna.GProMUT_hash_handle.ByReference hh_head, int count, int expand_mult) {
		super();
		this.hh_head = hh_head;
		this.count = count;
		this.expand_mult = expand_mult;
		write();
	}
	public static class ByReference extends GProMUT_hash_bucket implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMUT_hash_bucket implements Structure.ByValue {
		
	};
}
