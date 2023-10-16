var Model = {

  'sse'  : {}
};


var View = {

	'render_msg' : function(msg) {
	
		var html = $('#chatLog').html() + "<div style='background-color:#DEDEDE;padding:3px'>"
			+ "<div style='display:table;padding:3px;position:relative;left:30px;top:-40px;background-color:#cdcdcd;'>"
			+ "<img src='" + msg.img + "' style='width:48px;height:auto;margin:3px;'>"
			+ "<b>" + msg.login + "</b></div>" + msg.msg + "</div><br><br>";

			$('#chatLog').html(html);
		
		$("#chatLog").scrollTop($("#chatLog")[0].scrollHeight);
	}
};

var Controller = {

  'init' : function() {
  
	Model.sse = new EventSource("/sse");
  	Model.sse.onclose = function() {};
  	Model.sse.onmessage = Controller.onMsg;
  },
  
  'onMsg' : function(e) {

	//console.log(e);
  	View.render_msg(JSON.parse(e.data));
  },
  
  'send_msg' : async function(post) {
  
  	var msg = {
  		'msg' : post
	};
	  
	try {
		const response = await fetch('https://' + location.host + "/msg", {
			method: 'POST', 
			body: JSON.stringify(msg), 
			headers: {
				'Content-Type': 'application/json'
			},
			credentials: 'same-origin'
		});
		const json = await response.json();
		//console.log('Success:', JSON.stringify(json));
	} catch (error) {
		console.error('Error:', error);
	}
  	
  },
  
  'onSubmit' : function(el) {
  
  	var msg = $('#chatMsg').val();
  	//alert("MSG:" + msg);
  	Controller.send_msg(msg);
  	return false;
  }
};



(function($)  {

	Controller.init();
	
})(jQuery);