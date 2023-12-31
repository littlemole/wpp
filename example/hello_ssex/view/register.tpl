
<div id="title" class="smooth" hx-swap-oob="true">
<div class="frame">
<div class="title">
    <h1><!--#i18n key="register.title.headline" --></h1>
    <p><!--#i18n key="register.title.desc" --></p>
</div>
</div>



<!-- The flexible grid (content) -->
<div class="row">

  <!--#include virtual="/inc/navpanel.inc" -->

  <div id="sitedivider"></div>
  <div id="main" class="main">
      <h2><!--#i18n key="register.main.headline" --></h2>

				<h4>
					<h2><!--#i18n key="register.main.desc" --></h2>
				</h4>
				<p><!--#i18n key="register.main.fields" --></p>
				<p><b style="color:red">{{errorMsg}}</b></p>
				<form action="/register" method="POST" hx-push-url="true" hx-select="#main" hx-post="/register" hx-target="#main" hx-swap="outerHTML settle:2s" >
					<table style="width:50%;margin:5px auto;">
						<tr>
							<td>name</td>
							<td><input type="text" name="username"></td>
						</tr>
						<tr>
							<td>login</td>
							<td><input type="text" name="login"></td>
						</tr>
						<tr>
							<td>avatar_url</td>
							<td><input type="text" name="avatar_url" value=""></td>
						</tr>
						<tr>
							<td>password</td>
							<td><input type="password" name="pwd"></td>
						</tr>
						<tr>
							<td colspan=2><input type="submit" value="register"></td>
						</tr>
					</table>
				</form>

  </div>
</div>
</div>



