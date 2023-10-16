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
    <h1><!--#i18n key="login.title.headline" --></h1>
    <p><!--#i18n key="login.title.desc" --></p>
</div>
</div>


<!-- The flexible grid (content) -->
<div class="row">

  <!--#include virtual="/inc/navpanel.inc" -->

  <div id="sitedivider"></div>
  <div class="main">
      <h2><!--#i18n key="login.main.headline" --></h2>

	  			<p><!--#i18n key="login.main.desc" -->
				  <a href="/register"><!--#i18n key="login.main.desc.link" --></a>
				</p>
				<p><b style="color:red">{{errorMsg}}</b></p>
				<form action="/login" method="POST">
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

<!-- Footer -->
<!--#include virtual="/inc/footer.inc" -->


</body>
</html>

