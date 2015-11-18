# UPDATE: Microsoft has a Fix
  
Microsoft has published a fix, using a diferent approch as used here.
They are limiting the number of post parameters to exactly 1000.

To install this update, use Windows Update or visit the link belowe:

http://technet.microsoft.com/en-us/security/bulletin/ms11-100.mspx

This project however can be used for study of the IIS Modules and for future
problems.
  
  
# Old README

## The Problem

This is a simple IIS 7 module to avoid the zero day vulnerability which targets
string hash colisions discovered by Alexander "alech" Klink and Julian "zeri"
Wälde [1] which is described in a security advisory [2].

There was a security advisory [2] of Microsoft describing the problem and a
blog post explaing in details the problem [3].

## The Proposed Solution

This solution consists in expection the request input before ASP .net and
detecting duplicated form post hashes.

If the form post is "big" ann contains a duplicated hash then the request
is rejected.

I used some reference to implement this module. [5]

## Warning

This solution needs more work, I only released it now to share the discovery
with the community and to receive comments.

I will add more checks and check more data at input, it will be released
until the end of the week.

Please send any comments to <sergio@ginx.com.br>.

The vulnerability is better described in these links:

[1] http://events.ccc.de/congress/2011/Fahrplan/attachments/2007_28C3_Effective_DoS_on_web_application_platforms.pdf
[2] http://www.cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2011-3414
[3] http://technet.microsoft.com/en-us/security/advisory/2659883
[4] http://blogs.technet.com/b/srd/archive/2011/12/27/more-information-about-the-december-2011-asp-net-vulnerability.aspx
[5] http://learn.iis.net/page.aspx/169/develop-a-native-cc-module-for-iis/

