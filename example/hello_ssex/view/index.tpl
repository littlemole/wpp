

<div id="title" class="smooth" hx-swap-oob="true">
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
  <div id="main" class="main">


      <h2><!--#i18n key="index.main.headline" --></h2>

      <h5><!--#i18n key="index.main.greeting" --></h5>
      <p><!--#i18n key="index.main.logout" --> 
       <a href="/logout" hx-target="#main" hx-push-url="true" hx-select="#main" hx-swap="outerHTML" hx-get="/logout"><!--#i18n key="index.main.logout.link" --></a>. 
      </p>

      <h2><!--#i18n key="index.main.chat" --></h2>

	<form method="post" action="/msg" hx-post="/msg" hx-swap="none" >
	<table>
	<tr>
	<td>
		<div id="chatLog" hx-swap="beforeend" hx-ext="sse" sse-connect="/sse" sse-swap="message" class="" style="padding:40px 5px 5px 5px;border:1px solid #cdcdcd;overflow:scroll;height:320px;width:640px;"></div>
	</td>
	</tr>
	<tr>
	<td>
		<input type="text" class="text" id="chatMsg" name="chatMsg" />
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
</div>

