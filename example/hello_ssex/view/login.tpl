

<div id="title" class="smooth" hx-swap-oob="true">
<div class="frame">
<div class="title main">
    <h1><!--#i18n key="login.title.headline" --></h1>
    <p><!--#i18n key="login.title.desc" --></p>
</div>
</div>


<!-- The flexible grid (content) -->
<div class="row">

  <!--#include virtual="/inc/navpanel.inc" -->

  <div id="sitedivider"></div>
  <div id="main" class="main">
      <h2><!--#i18n key="login.main.headline" --></h2>

	  			<p><!--#i18n key="login.main.desc" -->
				  <a href="/register" hx-target="#main" hx-push-url="true" hx-select="#main" hx-swap="outerHTML" hx-get="/register" ><!--#i18n key="login.main.desc.link" --></a>
				</p>
				<p><b style="color:red">{{errorMsg}}</b></p>
				<form action="/login" method="POST" hx-push-url="true" hx-select="#main" hx-post="/login" hx-target="#main" hx-swap="outerHTML">
					<table style="width:50%;margin:5px auto;">
						<tr>
							<td>login</td>
							<td><input type="text" name="login"></td>
						</tr>
						<tr>
							<td>password</td>
							<td><input type="password" name="pwd"></td>
						</tr>
						<tr>
							<td colspan=2>
								<input type="submit" value="login">
							</td>						
						</tr>
					</table>
				</form>

  </div>
</div>
</div>
