/**
 * 
 */
package org.gprom.spark.session;

import org.apache.spark.SparkContext;
import org.apache.spark.sql.SparkSession;
import static org.apache.spark.sql.SparkSession.Builder;

import java.lang.reflect.Field;

import org.apache.spark.sql.SparkSessionExtensions;
import org.apache.spark.sql.internal.SessionState;
import org.apache.spark.sql.internal.SharedState;
import org.gprom.jdbc.jna.GProMWrapper;

import com.sun.istack.internal.logging.Logger;

import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;

import scala.Option;

/**
 * @author lord_pretzel
 *
 */
public class GProMSparkSession extends SparkSession {

	static Logger log = Logger.getLogger(GProMSparkSession.class); 
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

//	private SparkSession wrappedSession;
	private GProMWrapper w;
	
	/**
	 * @param sparkContext
	 * @param existingSharedState
	 * @param parentSessionState
	 * @param extensions
	 */
	public GProMSparkSession(SparkContext sparkContext,
			Option<SharedState> existingSharedState,
			Option<SessionState> parentSessionState,
			SparkSessionExtensions extensions) {
		super(sparkContext, existingSharedState, parentSessionState, extensions);
		initGpromWrapper();
	}

	/**
	 * @param sc
	 */
	public GProMSparkSession(SparkContext sc) {
		super(sc);
		initGpromWrapper();
	}

	/**
	 * @param wrappedSession2
	 */
	public GProMSparkSession(SparkSession wrappedSession) {
		super(wrappedSession.sparkContext());
		copyPrivateFields(wrappedSession);
		initGpromWrapper();
	}
	
	/**
	 * 
	 * @param s
	 */
	private void copyPrivateFields(SparkSession s) {
		Class<? extends SparkSession> sessionClass = s.getClass();
		
		try {
//			Field sparkContextField = sessionClass.getDeclaredField("sparkContext");
//			sparkContextField.setAccessible(true);
			Field existingSharedStateField = sessionClass.getDeclaredField("existingSharedState");
			existingSharedStateField.setAccessible(true);
			Field parentSessionStateField = sessionClass.getDeclaredField("parentSessionState");
			parentSessionStateField.setAccessible(true);
			Field extensionsField = sessionClass.getDeclaredField("extensions");
			extensionsField.setAccessible(true);
			Field sharedStateField = sessionClass.getDeclaredField("sharedState");
			sharedStateField.setAccessible(true);
			Field sessionStateField = sessionClass.getDeclaredField("sessionState");
			sessionStateField.setAccessible(true);
			Field sqlContextField = sessionClass.getDeclaredField("sqlContext");
			sqlContextField.setAccessible(true);
			Field confField = sessionClass.getDeclaredField("conf");
			confField.setAccessible(true);
			Field emptyDataFrameField = sessionClass.getDeclaredField("emptyDataFrame");
			emptyDataFrameField.setAccessible(true);
			Field catalogField = sessionClass.getDeclaredField("catalog");
			catalogField.setAccessible(true);
			
			existingSharedStateField.set(this,existingSharedStateField.get(s));
			parentSessionStateField.set(this,parentSessionStateField.get(s));
			extensionsField.set(this,extensionsField.get(s));
			sharedStateField.set(this,sharedStateField.get(s));
			sessionStateField.set(this,sessionStateField.get(s));
			sqlContextField.set(this,sqlContextField.get(s));
			confField.set(this,confField.get(s));
			emptyDataFrameField.set(this,emptyDataFrameField.get(s));
			catalogField.set(this,catalogField.get(s));
		} catch (Exception e) {
			
		}
		
		
//		implicits$module : 
//		org$apache$spark$internal$Logging$$log_ : Logger
	}
	
	/**
	 * 
	 */
	private void initGpromWrapper () {
		w = GProMWrapper.inst;
		w.setLogLevel(4);
	}

	/**
	 * 
	 * @author lord_pretzel
	 *
	 */
	private static class GProMBuilder extends Builder {
		
		/**
		 * creates a GProMSparkSession by creating a SparkSession and then copying it a GProMSparkSession
		 */
		public SparkSession getOrCreate() {
			SparkSession wrappedSession = super.getOrCreate();
			if (wrappedSession instanceof GProMSparkSession)
				return wrappedSession;
			
			GProMSparkSession gspark = new GProMSparkSession(wrappedSession);
			Option<SparkSession> defaultSession = SparkSession.getDefaultSession();
			if (defaultSession.getOrElse(null) == wrappedSession)
			{
				SparkSession.setDefaultSession(gspark);
			}
			return gspark;
		}
		
	}
	
	public static Builder builder() {
		return new GProMBuilder();
	}
	
	//   def sql(sqlText: String): DataFrame = {
    // Dataset.ofRows(self, sessionState.sqlParser.parsePlan(sqlText))
    // }
	@Override
	public Dataset<Row> sql (String sqlText) {
		String gpromSQL = sqlText;//TODO call GProM
		
		return super.sql(gpromSQL);
	}
	
	@Override
	public void close() {
		super.close();
		w.close();
	}
	
}
