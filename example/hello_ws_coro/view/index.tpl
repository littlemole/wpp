<!DOCTYPE html>
<html>
<head>
<!--#include virtual="/inc/header.inc" -->
</head>
<body>

<!-- Header navbar -->
<!--#include virtual="/inc/navbar.inc" -->

<div class="frame">
<div class="title">
    <h1><!--#i18n key="index.title.headline" --></h1>
    <p><!--#i18n key="index.title.desc" --></p>
</div>
</div>



<!-- The flexible grid (content) -->
<div class="row">

  <!--#include virtual="/inc/navpanel.inc" -->

  <div id="sitedivider"></div>
  <div class="main">
      <h2><!--#i18n key="index.main.headline" --></h2>

      <h5><!--#i18n key="index.main.greeting" --></h5>
      <p><!--#i18n key="index.main.logout" --> 
       <a href="/logout"><!--#i18n key="index.main.logout.link" --></a>. 
      </p>

      <h2><!--#i18n key="index.main.chat" --></h2>

	<form method="post" action="#" onSubmit="return Controller.onSubmit(this);">
	<table>
	<tr>
	<td>
		<div id="chatLog" class="" style="padding:40px 5px 5px 5px;border:1px solid #cdcdcd;overflow:scroll;height:320px;width:640px;"></div>
	</td>
	</tr>
	<tr>
	<td>
		<input type="text" class="text" id="chatMsg" />
	</td>
	</tr>
	<tr>
	<td>
		<input type="submit" value="<!--#i18n key='index.main.chat.send' -->" />
	</td>
	</tr>
	</table>
	</form>
  </div>
</div>

<!-- Footer -->
<!--#include virtual="/inc/footer.inc" -->

<script src="/js/jquery.min.js"></script>
<script src="/js/jquery.cookie.js"></script>
<script src="/js/mvc.js"></script>
</body>
</html>

