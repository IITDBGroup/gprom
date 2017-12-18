package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMUT_hash_handle extends GProMStructure {
	/** C type : GProMUT_hash_table* */
	public org.gprom.jdbc.jna.GProMUT_hash_table.ByReference tbl;
	/**
	 * prev element in app order<br>
	 * C type : void*
	 */
	public Pointer prev;
	/**
	 * next element in app order<br>
	 * C type : void*
	 */
	public Pointer next;
	/**
	 * previous hh in bucket order<br>
	 * C type : GProMUT_hash_handle*
	 */
	public GProMUT_hash_handle.ByReference hh_prev;
	/**
	 * next hh in bucket order<br>
	 * C type : GProMUT_hash_handle*
	 */
	public GProMUT_hash_handle.ByReference hh_next;
	/**
	 * ptr to enclosing struct's key<br>
	 * C type : void*
	 */
	public Pointer key;
	/** enclosing struct's key len */
	public int keylen;
	/** result of hash-fcn(key) */
	public int hashv;
	public GProMUT_hash_handle() {
		super();
	}
	public GProMUT_hash_handle(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("tbl", "prev", "next", "hh_prev", "hh_next", "key", "keylen", "hashv");
	}
	/**
	 * @param tbl C type : GProMUT_hash_table*<br>
	 * @param prev prev element in app order<br>
	 * C type : void*<br>
	 * @param next next element in app order<br>
	 * C type : void*<br>
	 * @param hh_prev previous hh in bucket order<br>
	 * C type : GProMUT_hash_handle*<br>
	 * @param hh_next next hh in bucket order<br>
	 * C type : GProMUT_hash_handle*<br>
	 * @param key ptr to enclosing struct's key<br>
	 * C type : void*<br>
	 * @param keylen enclosing struct's key len<br>
	 * @param hashv result of hash-fcn(key)
	 */
	public GProMUT_hash_handle(org.gprom.jdbc.jna.GProMUT_hash_table.ByReference tbl, Pointer prev, Pointer next, GProMUT_hash_handle.ByReference hh_prev, GProMUT_hash_handle.ByReference hh_next, Pointer key, int keylen, int hashv) {
		super();
		this.tbl = tbl;
		this.prev = prev;
		this.next = next;
		this.hh_prev = hh_prev;
		this.hh_next = hh_next;
		this.key = key;
		this.keylen = keylen;
		this.hashv = hashv;
		write();
	}
	public static class ByReference extends GProMUT_hash_handle implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMUT_hash_handle implements Structure.ByValue {
		
	};
}
