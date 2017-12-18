package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMUT_hash_table extends GProMStructure {
	/** C type : GProMUT_hash_bucket* */
	public org.gprom.jdbc.jna.GProMUT_hash_bucket.ByReference buckets;
	public int num_buckets;
	public int log2_num_buckets;
	public int num_items;
	/**
	 * tail hh in app order, for fast append<br>
	 * C type : GProMUT_hash_handle*
	 */
	public org.gprom.jdbc.jna.GProMUT_hash_handle.ByReference tail;
	/** hash handle offset (byte pos of hash handle in element */
	public int hho;
	/**
	 * in an ideal situation (all buckets used equally), no bucket would have<br>
	 * more than ceil(#items/#buckets) items. that's the ideal chain length.
	 */
	public int ideal_chain_maxlen;
	/**
	 * nonideal_items is the number of items in the hash whose chain position<br>
	 * exceeds the ideal chain maxlen. these items pay the penalty for an uneven<br>
	 * hash distribution; reaching them in a chain traversal takes >ideal steps
	 */
	public int nonideal_items;
	/**
	 * ineffective expands occur when a bucket doubling was performed, but <br>
	 * afterward, more than half the items in the hash had nonideal chain<br>
	 * positions. If this happens on two consecutive expansions we inhibit any<br>
	 * further expansion, as it's not helping; this happens when the hash<br>
	 * function isn't a good fit for the key domain. When expansion is inhibited<br>
	 * the hash will still work, albeit no longer in constant time.
	 */
	public int ineff_expands;
	/**
	 * ineffective expands occur when a bucket doubling was performed, but <br>
	 * afterward, more than half the items in the hash had nonideal chain<br>
	 * positions. If this happens on two consecutive expansions we inhibit any<br>
	 * further expansion, as it's not helping; this happens when the hash<br>
	 * function isn't a good fit for the key domain. When expansion is inhibited<br>
	 * the hash will still work, albeit no longer in constant time.
	 */
	public int noexpand;
	/** used only to find hash tables in external analysis */
	public int signature;
	public GProMUT_hash_table() {
		super();
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("buckets", "num_buckets", "log2_num_buckets", "num_items", "tail", "hho", "ideal_chain_maxlen", "nonideal_items", "ineff_expands", "noexpand", "signature");
	}
	public static class ByReference extends GProMUT_hash_table implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMUT_hash_table implements Structure.ByValue {
		
	};
}
