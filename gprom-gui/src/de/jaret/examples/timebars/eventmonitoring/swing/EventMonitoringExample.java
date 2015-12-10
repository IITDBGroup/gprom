/*
 *  File: EventMonitoringExample.java 
 *  Copyright (c) 2004-2007  Peter Kliem (Peter.Kliem@jaret.de)
 *  A commercial license is available, see http://www.jaret.de.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package de.jaret.examples.timebars.eventmonitoring.swing;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Point;
import java.awt.datatransfer.StringSelection;
import java.awt.dnd.DnDConstants;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragGestureRecognizer;
import java.awt.dnd.DragSource;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.List;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JFrame;
import javax.swing.JPopupMenu;
import javax.swing.Timer;

import de.jaret.examples.timebars.eventmonitoring.model.CollectingTimeBarNode;
import de.jaret.examples.timebars.eventmonitoring.model.EventInterval;
import de.jaret.examples.timebars.eventmonitoring.model.EventTimeBarRow;
import de.jaret.examples.timebars.eventmonitoring.model.ModelCreator;
import de.jaret.examples.timebars.eventmonitoring.swing.renderer.EventMonitorHeaderRenderer;
import de.jaret.examples.timebars.eventmonitoring.swing.renderer.EventRenderer;
import de.jaret.util.date.Interval;
import de.jaret.util.date.JaretDate;
import de.jaret.util.ui.timebars.TimeBarMarker;
import de.jaret.util.ui.timebars.TimeBarMarkerImpl;
import de.jaret.util.ui.timebars.TimeBarViewerDelegate;
import de.jaret.util.ui.timebars.model.HierarchicalTimeBarModel;
import de.jaret.util.ui.timebars.model.HierarchicalViewStateListener;
import de.jaret.util.ui.timebars.model.ISelectionRectListener;
import de.jaret.util.ui.timebars.model.ITimeBarChangeListener;
import de.jaret.util.ui.timebars.model.ITimeBarViewState;
import de.jaret.util.ui.timebars.model.TBRect;
import de.jaret.util.ui.timebars.model.TimeBarModel;
import de.jaret.util.ui.timebars.model.TimeBarNode;
import de.jaret.util.ui.timebars.model.TimeBarRow;
import de.jaret.util.ui.timebars.strategy.IIntervalSelectionStrategy;
import de.jaret.util.ui.timebars.swing.TimeBarViewer;
import de.jaret.util.ui.timebars.swing.renderer.DefaultHierarchyRenderer;
import de.jaret.util.ui.timebars.swing.renderer.DefaultTitleRenderer;

/**
 * Example showing events (non modificable).
 * 
 * @author Peter Kliem
 * @version $Id: EventMonitoringExample.java 1073 2010-11-22 21:25:33Z kliem $
 */
public class EventMonitoringExample {
    TimeBarViewer _tbv;
    TimeBarMarkerImpl _tm;
    
    private final static boolean HIERARCHICAL = false;

    public static void main(String[] args) {
        EventMonitoringExample example = new EventMonitoringExample();
        example.run();
    }

    public EventMonitoringExample() {
    }

    public void run() {
        JFrame f = new JFrame(EventMonitoringExample.class.getName());
        f.setSize(1400, 600);
        f.getContentPane().setLayout(new BorderLayout());
        f.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

        HierarchicalTimeBarModel hierarchicalModel = ModelCreator.createHierarchicalModel();
        final TimeBarModel flatModel = ModelCreator.createFlatModel();
     
        _tbv = new TimeBarViewer();
        
        
        if (HIERARCHICAL) {
            _tbv.setModel(hierarchicalModel);
            _tbv.setHierarchyRenderer(new DefaultHierarchyRenderer());
            _tbv.setHierarchyWidth(100);
                 } else {
            _tbv.setModel(flatModel);
            _tbv.setHierarchyWidth(0);
        }
        _tbv.setTimeScalePosition(TimeBarViewer.TIMESCALE_POSITION_TOP);
        _tbv.setYAxisWidth(100);
        f.getContentPane().add(_tbv, BorderLayout.CENTER);

        // allow marker grabbing in the diagram area
        _tbv.setMarkerDraggingInDiagramArea(true);

        // enable region selection
        _tbv.setRegionRectEnable(true);
        
        // draw row grid
        _tbv.setDrawRowGrid(true);
        
        // setup header renderer
        _tbv.setHeaderRenderer(new EventMonitorHeaderRenderer());
        
        
        // set a name for the viewer and setup the default title renderer
        _tbv.setName("Monitor");
        _tbv.setTitleRenderer(new DefaultTitleRenderer());
        
        
        // selection strategy: shortest first
        _tbv.getDelegate().setIntervalSelectionStrategy(new IIntervalSelectionStrategy() {
            public Interval selectInterval(List<Interval> intervals) {
                Interval result = null; 
                for (Interval interval : intervals) {
                    if (result == null || interval.getSeconds()<result.getSeconds()) {
                        result = interval;
                    }
                }
                return result;
            }
        });
        
        
        if (HIERARCHICAL) {
        	int i = 0;
            // set the overlap drawing property for the nodes on the second level
            for (TimeBarNode node : hierarchicalModel.getRootNode().getChildren()) {
                _tbv.getTimeBarViewState().setDrawOverlapping(node, true);
            }

            // add a listener that toggles the collect childIntervals property of expanded/collapsed nodes
            _tbv.getHierarchicalViewState().addHierarchicalViewstateListener(new HierarchicalViewStateListener() {
                public void nodeExpanded(TimeBarNode node) {
                    if (node instanceof CollectingTimeBarNode) {
                        CollectingTimeBarNode ctbn = (CollectingTimeBarNode) node;
                        ctbn.setCollectChildIntervals(false);
                    }
                }

                public void nodeFolded(TimeBarNode node) {
                    if (node instanceof CollectingTimeBarNode) {
                        CollectingTimeBarNode ctbn = (CollectingTimeBarNode) node;
                        ctbn.setCollectChildIntervals(true);
                    }
                }
            });
        } else {
            // in general draw overlapping
            _tbv.setDrawOverlapping(true);
            
            // allow different row heights
            _tbv.getTimeBarViewState().setUseVariableRowHeights(true);
            
            // add a double click listener for checking on the header
            _tbv.addMouseListener(new MouseAdapter() {

                public void mouseClicked(MouseEvent e) {
                	   Point origin2 = e.getPoint();
                	TimeBarRow row2 = _tbv.getRowForXY(origin2.x, origin2.y);
//                	System.out.println(row2.getIntervals().get(0).getBegin());
                	//在这里添加弹出窗口界面。。。
                	
                	EventInterval interval = (EventInterval) flatModel.getRow(0).getIntervals().get(0);
                	  System.out.println(interval.getTitle());
                	
                	//                	System.out.println("首都发生的加法阿拉山口的加法拉开圣诞节");
                    if (e.getClickCount() == 2) {
                    	Point origin = e.getPoint();
                        if (_tbv.getDelegate().getYAxisRect().contains(origin)) {
                            TimeBarRow row = _tbv.getRowForXY(origin.x, origin.y);
                            if (row != null) {
                                if (row instanceof EventTimeBarRow) {
                                    EventTimeBarRow erow = (EventTimeBarRow) row;
                                    if (!erow.isExpanded()) {
                                        // expand
                                        _tbv.getTimeBarViewState().setDrawOverlapping(row, false);
                                        _tbv.getTimeBarViewState().setRowHeight(row, calculateRowHeight(_tbv.getDelegate(), _tbv.getTimeBarViewState(), row));
                                        erow.setExpanded(true);
                                    } else {
                                    	
                                        // fold
                                        _tbv.getTimeBarViewState().setDrawOverlapping(row, true);
                                        _tbv.getTimeBarViewState().setRowHeight(row, _tbv.getTimeBarViewState().getDefaultRowHeight());
                                        erow.setExpanded(false);
                                        }
                                }
                            }
                            
                        }
                    }
                }

                /**
                 * Calculate the optimal row height
                 * @param delegate
                 * @param timeBarViewState
                 * @param row
                 * @return
                 */
                public int calculateRowHeight(TimeBarViewerDelegate delegate,
                        ITimeBarViewState timeBarViewState, TimeBarRow row) {
                    int maxOverlap = timeBarViewState.getDefaultRowHeight();
                    int height = delegate.getMaxOverlapCount(row) * maxOverlap;
                    return height;
                }

                
                
            });
            
            
        }
        
        // change listener
        _tbv.addTimeBarChangeListener(new ITimeBarChangeListener() {

            public void intervalChangeCancelled(TimeBarRow row, Interval interval) {
                System.out.println("CHANGE CANCELLED " + row + " " + interval);
            }

            public void intervalChangeStarted(TimeBarRow row, Interval interval) {
                System.out.println("CHANGE STARTED " + row + " " + interval);
            }

            public void intervalChanged(TimeBarRow row, Interval interval, JaretDate oldBegin, JaretDate oldEnd) {
                System.out.println("CHANGE DONE " + row + " " + interval);
            }

            public void intervalIntermediateChange(TimeBarRow row, Interval interval, JaretDate oldBegin,
                    JaretDate oldEnd) {
                System.out.println("CHANGE INTERMEDIATE " + row + " " + interval);
            }

            public void markerDragStarted(TimeBarMarker marker) {
                System.out.println("Marker drag started "+marker);
            }

            public void markerDragStopped(TimeBarMarker marker) {
                System.out.println("Marker drag stopped "+marker);
            }

        });

        
        
// sample property listener        
//        _tbv.addPropertyChangeListener(_tbv.PROPERTYNAME_STARTDATE, new PropertyChangeListener(){
//
//            @Override
//            public void propertyChange(PropertyChangeEvent evt) {
//                System.out.println("Start changed to "+evt.getNewValue());
//            }
//            
//        });
        
        // Do not allow any modifications - do not add an interval modificator!
        // _tbv.addIntervalModificator(new DefaultIntervalModificator());

        // do not allow row selections
        _tbv.getSelectionModel().setRowSelectionAllowed(false);

        // register additional renderer
        _tbv.registerTimeBarRenderer(EventInterval.class, new EventRenderer());

        // add a marker
        _tm = new TimeBarMarkerImpl(true, _tbv.getModel().getMinDate().copy().advanceHours(3));
        _tm.setDescription("Timebarmarker");
        _tbv.addMarker(_tm);

        // do not show the root node
        _tbv.setHideRoot(true);

        // add a popup menu for EventIntervals
        Action action = new AbstractAction("IntervalAction") {
            public void actionPerformed(ActionEvent e) {
                System.out.println("run " + getValue(NAME));
            }
        };
        JPopupMenu pop = new JPopupMenu("Operations");
        pop.add(action);
        _tbv.registerPopupMenu(EventInterval.class, pop);

        // add a popup menu for the body
        final Action bodyaction = new AbstractAction("BodyAction") {
            public void actionPerformed(ActionEvent e) {
                System.out.println("run " + getValue(NAME));
            }
        };
        pop = new JPopupMenu("Operations");
        pop.add(bodyaction);
        pop.add(new RunMarkerAction(_tbv));
        
        // add the zoom action
        pop.add(new ZoomAction(_tbv));
        // add the rem selection action
        pop.add(new ResetRegionSelectionAction(_tbv));
        
        _tbv.setBodyContextMenu(pop);

        // sample: check enablement of action in a popup
//        pop.addPopupMenuListener(new PopupMenuListener() {
//            
//            public void popupMenuWillBecomeVisible(PopupMenuEvent e) {
//                System.out.println(_tbv.getPopUpInformation().getLeft());
//                System.out.println(_tbv.getPopUpInformation().getRight().toDisplayString());
//                if (_tbv.getPopUpInformation().getRight().getHours()>9) {
//                    bodyaction.setEnabled(false);
//                } else {
//                    bodyaction.setEnabled(true);
//                }
//            }
//            
//            public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {
//                // TODO Auto-generated method stub
//                
//            }
//            
//            public void popupMenuCanceled(PopupMenuEvent e) {
//                // TODO Auto-generated method stub
//                
//            }
//        });
        
        
        
        // add a popup menu for the hierarchy
        action = new AbstractAction("HierarchyAction") {
            public void actionPerformed(ActionEvent e) {
                System.out.println("run " + getValue(NAME));
            }
        };
        pop = new JPopupMenu("Operations");
        pop.add(action);
        _tbv.setHierarchyContextMenu(pop);

        // add a popup menu for the header
        action = new AbstractAction("HeaderAction") {
            public void actionPerformed(ActionEvent e) {
                System.out.println("run " + getValue(NAME));
            }
        };
        pop = new JPopupMenu("Operations");
        pop.add(action);
        _tbv.setHeaderContextMenu(pop);

        // add a popup menu for the time scale
        action = new AbstractAction("TimeScaleAction") {
            public void actionPerformed(ActionEvent e) {
                System.out.println("run " + getValue(NAME));
            }
        };
        pop = new JPopupMenu("Operations");
        pop.add(action);
        _tbv.setTimeScaleContextMenu(pop);

        // add a popup menu for the title area
        action = new AbstractAction("TitleAction") {
            public void actionPerformed(ActionEvent e) {
                System.out.println("run " + getValue(NAME));
            }
        };
        pop = new JPopupMenu("Operations");
        pop.add(action);
        _tbv.setTitleContextMenu(pop);

        // add dnd support
        DragSource dragSource = DragSource.getDefaultDragSource();
        DragGestureListener dgl = new TimeBarViewerDragGestureListener();
        DragGestureRecognizer dgr = dragSource.createDefaultDragGestureRecognizer(_tbv._diagram,
                DnDConstants.ACTION_COPY, dgl);

        // add the control panel
        EventMonitoringControlPanel cp = new EventMonitoringControlPanel(_tbv, _tm, 100); // TODO
        f.getContentPane().add(cp, BorderLayout.SOUTH);

        
        
        // make sure the marker is in a certain area when zooming
        // relative display range the marker should be in after zooming
        final double min = 0.3;
        final double max = 0.7;
        
//      _tbv.addPropertyChangeListener(_tbv.PROPERTYNAME_PIXELPERSECOND, new PropertyChangeListener(){
//                    public void propertyChange(PropertyChangeEvent evt) {
//                        // if not displayed set the viewer to display the marker at the min position
//                        if (!isInRange(_tm.getDate(), min, max)) {
//                            int secondsDisplayed = _tbv.getSecondsDisplayed();
//                            JaretDate startDate = _tm.getDate().copy().advanceSeconds(-min*secondsDisplayed);
//     //                       _tbv.setStartDate(startDate);
//                        }
//                    }
//                    
//                });
        
        
        
        
        // go!
        f.setVisible(true);
    }

    public void setEndDate(TimeBarViewer tbv, JaretDate endDate) {
        int secondsDisplayed = tbv.getSecondsDisplayed();
        JaretDate startDate = endDate.copy().advanceSeconds(-secondsDisplayed);
        tbv.setStartDate(startDate);
    }

    
    boolean isInRange(JaretDate date, double min, double max) {
        int secondsDisplayed = _tbv.getSecondsDisplayed();
        JaretDate minDate = _tbv.getStartDate().copy().advanceSeconds(min*secondsDisplayed);
        JaretDate maxDate = _tbv.getStartDate().copy().advanceSeconds(max*secondsDisplayed);
        return minDate.compareTo(date)>0 && maxDate.compareTo(date)<0;
    }
    
    class TimeBarViewerDragGestureListener implements DragGestureListener {
        public void dragGestureRecognized(DragGestureEvent e) {
            Component c = e.getComponent();
            System.out.println("component " + c);
            System.out.println(e.getDragOrigin());

            boolean markerDragging = _tbv.getDelegate().isMarkerDraggingInProgress();
            if (markerDragging) {
                return;
            }

            List<Interval> intervals = _tbv.getDelegate().getIntervalsAt(e.getDragOrigin().x, e.getDragOrigin().y);
            if (intervals.size() > 0) {
                Interval interval = intervals.get(0);
                e.startDrag(null, new StringSelection("Drag " + ((EventInterval) interval).getTitle()));
                return;
            }
            Point origin = e.getDragOrigin();
            if (_tbv.getDelegate().getYAxisRect().contains(origin)) {
                TimeBarRow row = _tbv.getRowForXY(origin.x, origin.y);
                if (row != null) {
                    e.startDrag(null, new StringSelection("Drag ROW " + row));
                }
            }

        }
    }
    
    
    
    /**
     * Simple zoom action.
     * @author kliem
     * @version $Id: EventMonitoringExample.java 1073 2010-11-22 21:25:33Z kliem $
     */
    class ZoomAction extends AbstractAction implements ISelectionRectListener{
        TimeBarViewer _tbv;
        public ZoomAction(TimeBarViewer tbv) {
            super("Zoom to selection");
            _tbv = tbv;
            setEnabled(false);
            _tbv.addSelectionRectListener(this);
        }
        
        
        public void actionPerformed(ActionEvent e) {
            if (_tbv.getRegionRect() != null) {
                TBRect tbrect = _tbv.getRegionRect();
                JaretDate startDate = tbrect.startDate;
                int seconds = tbrect.endDate.diffSeconds(tbrect.startDate);
                int pixel = _tbv.getDelegate().getDiagramRect().width;
                double pps = ((double) pixel) /((double)seconds);
                _tbv.clearRegionRect();
                _tbv.setPixelPerSecond(pps);
                _tbv.setStartDate(startDate);
                // TODO row scaling
            }
        }


        public void regionRectChanged(TimeBarViewerDelegate delegate, TBRect tbrect) {
            setEnabled(true);
        }


        public void regionRectClosed(TimeBarViewerDelegate delegate) {
            setEnabled(false);
        }


        public void selectionRectChanged(TimeBarViewerDelegate delegate, JaretDate beginDate, JaretDate endDate,
                List<TimeBarRow> rows) {
            // TODO Auto-generated method stub
            
        }

        public void selectionRectClosed(TimeBarViewerDelegate delegate) {
            // TODO Auto-generated method stub
            
        }
        
    }
    
    class ResetRegionSelectionAction extends AbstractAction {
        TimeBarViewer _tbv;
        public ResetRegionSelectionAction(TimeBarViewer tbv) {
            super("Remove selection");
            _tbv = tbv;
        }
        public void actionPerformed(ActionEvent e) {
            _tbv.clearRegionRect();
        }
        
    }
    class RunMarkerAction extends AbstractAction {
        TimeBarViewer _tbv;
        public RunMarkerAction(TimeBarViewer tbv) {
            super("Run marker");
            _tbv = tbv;
        }
        public void actionPerformed(ActionEvent e) {
            _tm.setDate(_tbv.getModel().getMinDate().copy());
            
            final Timer timer = new Timer(40, null);
            ActionListener al = new ActionListener() {

                public void actionPerformed(ActionEvent e) {
                    _tm.setDate(_tm.getDate().copy().advanceMillis(40));
                    if (_tm.getDate().compareTo(_tbv.getModel().getMaxDate())>0) {
                        timer.stop();
                    }
                }
            };
            timer.addActionListener(al);
            timer.setRepeats(true);
            timer.setDelay(40);
            timer.start();
        }
        
    }
    
    
}
