$(function () {
	Array.prototype.contains = function(obj) {
	    var i = this.length;
	    while (i--) {
	        if (this[i] === obj) {
	            return true;
	        }
	    }
	    return false;
	}

  	$.getJSON('reads.json', function(json){
	    console.log(json);

      var MAX_X = 1600;
      var x_axis = [];
      for(var i = 0; i <= MAX_X; i++) {
        if(i % 5 == 0) x_axis.push(i);
      }

		  var r_bytes = [0, 0];
		  var e_bytes = [0, 0];
		  var h_bytes = [0, 0];

	    for (var fanout in json) {
	  		if (json.hasOwnProperty(fanout)) {
				
				var fanoutResults = json[fanout];
				var title = getTitle(fanout);
				
				var r = [];
				var e = [];
				var h = [];
				var c = []; // convergences


	  			for (var i = 0; i < fanoutResults.length; i++) {
	  				var result = fanoutResults[i];
	  				console.log(Math.max.apply(null, result['convergences']));
	  				var key = result['key'];
	  				var keyParts = key.split('_');
	  				var protocolKey = keyParts[0];
	  				var withDeltas = keyParts[2] == "d";

	  				var name = getName(protocolKey);
	  				var color = getColor(protocolKey);
	  				var dashStyle = getDashStyle(withDeltas);
	  				var values = result['values'];
	  				var convergences = result['convergences'];

	  				var otherSerieData = [];
	  				var time = 0;
	  				
	  				for (var j = 0; j < values.length; j++) {
	  					var value = values[j];
	  					if(convergences.contains(time)) {
	  						otherSerieData.push(
		  						{
	                				y: value,
	                				marker: {
	                    				 symbol: 'circle'
	                				}
	            				}
            				);
	  					}
	  					else otherSerieData.push(null);
	  					time += 5;
	  				}

	  				c.push({
	  					color: color,
	  					marker: {enabled: true},
	  					showInLegend: false,
	  					data: otherSerieData,
	  				});

	  				var serie = {
	  					name: name,
	  					color: color,
	  					dashStyle: dashStyle,
	  					marker: {enabled : false},
	  					showInLegend: withDeltas,
	  					data: values
	  				};

	  				var total = values.slice(-1)[0];

	  				switch(protocolKey) {
	  					case "r":
	  						r.push(serie);
	  						if(keyParts[1] != "f") {
	  							if(withDeltas) r_bytes[0] += total;
	  							else r_bytes[1] += total;
	  						}
	  						break;
	  					case "e":
	  						e.push(serie);
	  						if(withDeltas) e_bytes[0] += total;
	  						else e_bytes[1] += total;
	  						break;
	  					case "h":
	  						h.push(serie);
	  						if(withDeltas) h_bytes[0] += total;
	  						else h_bytes[1] += total;
	  						break;
	  					default:
	  						console.log("what?");
	  						break;
	  				}
	  			}

	  			var series = [];
				for (var i = 0; i < r.length; i++) series.push(r[i]);
				for (var i = 0; i < e.length; i++) series.push(e[i]);
				for (var i = 0; i < h.length; i++) series.push(h[i]);
				for (var i = 0; i < c.length; i++) series.push(c[i]);

				console.log(series);
				draw('#container-' + fanout, title, series, x_axis);


	  		}
		}

		var deltas = r_bytes[0] + e_bytes[0] + h_bytes[0]; 
		var full = r_bytes[1] + e_bytes[1] + h_bytes[1];

		console.log("average: " + round(deltas * 100 / full));
		console.log("ring: " + round(r_bytes[0] * 100 / r_bytes[1]));
		console.log("erdos: " + round(e_bytes[0] * 100 / e_bytes[1]));
		console.log("hyparview: " + round(h_bytes[0] * 100 / h_bytes[1]));

	});
});

function round(num) {
	return Math.round(num * 100) / 100;
}

function getTitle(fanoutKey) {
	switch(fanoutKey) {
		case "f1":
			return "Fanout 1";
		case "f2":
			return "Fanout 2";
		case "f":
			return "Flooding";
	}
}

function getName(protocolKey){
	switch(protocolKey){
		case "h":
			return "HyParView";
		case "e":
			return "Erdős–Rényi";
		case "r":
			return "Ring";
	}
}

function getColor(protocolKey) {
	switch(protocolKey){
		case "h":
			return "#434348";
		case "e":
			return "#7cb5ec";
		case "r":
			return "#90ed7d";
	}
}

function getDashStyle(withDeltas) {
	return withDeltas ? "LongDash" : "Solid";
}

function draw(context, title, series, x_axis) {
    $(context).highcharts({
        title: {
            text: title,
            x: -20 //center
        },
        credits: {
        	enabled: false
        },
        xAxis: {
          categories: x_axis,
        	title: {
        		text: 'Time (second)'
        	},
        	//tickInterval: 5
        },
        yAxis: {
            title: {
                text: 'State (byte)'
            },
            plotLines: [{
                value: 0,
                width: 1,
                color: '#808080'
            }],
            minRange: 800000,
            max: 800000
        },
        legend: {
            layout: 'vertical',
            align: 'right',
            verticalAlign: 'middle',
            borderWidth: 0
        },
        exporting: {
        	width: 5000
        },
        series: series
    });
}
