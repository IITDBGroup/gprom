package org.gprom.jdbc.jna;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

public abstract class GProMStructure extends Structure 
{
	 public GProMStructure(Pointer address) { 
	    super(address); 
	    read();
	 } 
	 public GProMStructure() { 
	    super();
	 } 
	 
	 /*
	  * The JVM GC was collecting some of the nodes in operator 
	  * trees even though we still had a reference to the operator 
	  * tree root.  This subsequently was causing crashes in the
	  * native library and crashing the JVM.  
	  * I have not determined what the root cause of 
	  * this issue is yet but this is a hack to prevent the GC 
	  * from collecting this until we are finished with it.  
	  * 
	  * For now, when constructing GProM operator trees from the 
	  * JVM side use this idom:
	  * 
	  * synchronized(GProM_JNA.GC_LOCK){
	  *  //do operator construction and translation/rewrites here
	  * } 
	  * 
	  * I believe the issue may be related to this JNA issue:
	  * https://github.com/java-native-access/jna/issues/664
	  * 
	  */
	 private void keepAlive() {
		 //System.out.println(this.getClass().getCanonicalName()+"@"+ this.hashCode() +" T:" +Thread.currentThread().getId()+ " --> delaying GC: " + System.nanoTime() );
		 synchronized(GProM_JNA.GC_LOCK){
			 //System.out.println(this.getClass().getCanonicalName()+"@"+ this.hashCode() +" T:" +Thread.currentThread().getId()+ " --> allow GC to proceede: " + System.nanoTime());
		 }
	 }
	 @Override
     protected void finalize() throws Throwable {
		 keepAlive();
		 //System.out.println(this.getClass().getCanonicalName()+"@"+ this.hashCode() +" T:" +Thread.currentThread().getId()+ " --> finalize: " + System.nanoTime());
         super.finalize();
     }
	
}
