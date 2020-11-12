/*
 *  File: EventMonitoringControlPanel.java 
 *  Copyright (c) 2004-2008  Peter Kliem (Peter.Kliem@jaret.de)
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

import java.awt.Button;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import de.jaret.util.ui.timebars.TimeBarMarkerImpl;
import de.jaret.util.ui.timebars.swing.TimeBarViewer;
import de.jaret.util.ui.timebars.swing.renderer.BoxTimeScaleRenderer;
import de.jaret.util.ui.timebars.swing.renderer.DefaultGridRenderer;
import de.jaret.util.ui.timebars.swing.renderer.DefaultTimeScaleRenderer;

/**
 * Control panel for the event monitoring example.
 * 
 * @author Peter Kliem
 * @version $Id: EventMonitoringControlPanel.java 974 2009-12-22 22:15:29Z kliem $
 */
public class EventMonitoringControlPanel extends JPanel {
    TimeBarViewer _viewer;
    JSlider _timeScaleSlider;
    JSlider _rowHeigthSlider;
    JComboBox _sorterCombo;
    JComboBox _filterCombo;
    JComboBox _intervalFilterCombo;
    TimeBarMarkerImpl _marker;
    JButton _freisetzenButton;

    public EventMonitoringControlPanel(TimeBarViewer viewer, TimeBarMarkerImpl marker, int initalSecondsDisplayed) {
        _viewer = viewer;
        _marker = marker;
        this.setPreferredSize(new Dimension(1000, 100));
        setLayout(new FlowLayout());
        createControls(initalSecondsDisplayed);
    }

    /**
     * 
     */
    private void createControls(int initialSeconds) {
        final double min = 1; // minimum value for seconds displayed
        final double max = 3 * 365 * 24 * 60 * 60; // max nummber of seconds displayed (3 years in seconds)
        final double slidermax = 1000; // slider maximum (does not really matter)
        _timeScaleSlider = new JSlider(0, (int) slidermax);

        _timeScaleSlider.setPreferredSize(new Dimension(_timeScaleSlider.getPreferredSize().width * 4, _timeScaleSlider
                .getPreferredSize().height));
        add(_timeScaleSlider);

        final double b = 1.0 / 100.0; // additional factor to reduce the absolut values in the exponent
        final double faktor = (min - max) / (1 - Math.pow(2, slidermax * b)); // factor for the exp function
        final double c = (min - faktor);

        int initialSliderVal = calcInitialSliderVal(c, b, faktor, initialSeconds);
        _timeScaleSlider.setValue((int) (slidermax- initialSliderVal));

        final JCheckBox centeredZoomCheck = new JCheckBox("Zoom around center (when no marker is not visible");
        add(centeredZoomCheck);

        _timeScaleSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                final double x = slidermax - (double) _timeScaleSlider.getValue(); // reverse x
                double seconds = c + faktor * Math.pow(2, x * b); // calculate the seconds to display
                if (_viewer.isDisplayed(_marker.getDate())) {
                    _viewer.setSecondsDisplayed((int) seconds, _marker.getDate());
                } else {
                    _viewer.setSecondsDisplayed((int) seconds, centeredZoomCheck.isSelected());
                }
            }
        });

        _rowHeigthSlider = new JSlider(10, 300);
        _rowHeigthSlider.setValue(_viewer.getRowHeight());
        _rowHeigthSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                _viewer.setRowHeight(_rowHeigthSlider.getValue());
            }
        });
       

        final JCheckBox optScrollingCheck = new JCheckBox("Optimize scrolling");
        optScrollingCheck.setSelected(_viewer.getOptimizeScrolling());
        optScrollingCheck.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                _viewer.setOptimizeScrolling(optScrollingCheck.isSelected());
            }
        });
        add(optScrollingCheck);
        Button startBt = new Button("StartTime");
        add(startBt);
        Button endBt = new Button("EndTime");
        add(endBt);
        Button searchBt = new Button("Filter");
        add(searchBt);
        
        final JCheckBox markerInDiagramAreaCheck = new JCheckBox("Allow Marker drag in DiagramArea");
        markerInDiagramAreaCheck.setSelected(_viewer.getMarkerDraggingInDiagramArea());
        markerInDiagramAreaCheck.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                _viewer.setMarkerDraggingInDiagramArea(markerInDiagramAreaCheck.isSelected());
            }
        });
        add(markerInDiagramAreaCheck);

        final JCheckBox boxTSRCheck = new JCheckBox("BoxTimeScaleRenderer");
        boxTSRCheck.setSelected(_viewer.getTimeScaleRenderer() instanceof BoxTimeScaleRenderer);
        boxTSRCheck.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (boxTSRCheck.isSelected()) {
                    BoxTimeScaleRenderer btscr = new BoxTimeScaleRenderer();
                    _viewer.setTimeScaleRenderer(btscr);
                    if (_viewer.getGridRenderer() instanceof DefaultGridRenderer) {
                        ((DefaultGridRenderer) _viewer.getGridRenderer()).setTickProvider(btscr);
                    }
                } else {
                    DefaultTimeScaleRenderer dtscr = new DefaultTimeScaleRenderer();
                    _viewer.setTimeScaleRenderer(dtscr);
                }
            }
        });
        add(boxTSRCheck);
        final JCheckBox regionCheck = new JCheckBox("Enable region select");
        regionCheck.setSelected(_viewer.getRegionRectEnable());
        regionCheck.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                _viewer.setRegionRectEnable(regionCheck.isSelected());
            }
        });
        add(regionCheck);

        final JCheckBox uniformHeightCheck = new JCheckBox("Use uniform height");
        uniformHeightCheck.setSelected(_viewer.getUseUniformHeight());
        uniformHeightCheck.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                _viewer.setUseUniformHeight(uniformHeightCheck.isSelected());
            }
        });
        add(uniformHeightCheck);
    }

    private int calcInitialSliderVal(double c, double b, double faktor, int seconds) {

        double x = 1 / b * log2((seconds - c) / faktor);

        return (int) x;
    }

    private double log2(double a) {
        return Math.log(a) / Math.log(2);
    }

}