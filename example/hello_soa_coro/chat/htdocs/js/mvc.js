var Model = {

  'sid'  : ''
};

var Controller = {

  'init' : function() {
  
  	Model.sid = Cookies.get('repro_web_sid');
  	
  	if(Model.sid)
  	{
  		Controller.start_ws();
  	}
  },
  
  'start_ws' : function() {
  
  	Model.ws = new WebSocket('wss://localhost:9876/ws');
  	Model.ws.onclose = function() {};
  	Model.ws.onmessage = Controller.onMsg;
  },
  
  'onMsg' : function(e) {
  
//  	alert(JSON.stringify(e.data));
  	View.render_msg(JSON.parse(e.data));
  },
  
  'send_msg' : function(post) {
  
  	var msg = {
  		'msg' : post,
  		'sid' : Model.sid
  	};
  	
  	Model.ws.send(JSON.stringify(msg));
  },
  
  'onSubmit' : function(el) {
  
  	var msg = $('#chatMsg').val();
  	//alert("MSG:" + msg);
  	Controller.send_msg(msg);
  	return false;
  }
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


(function($) 
{
	Controller.init();

})(jQuery);