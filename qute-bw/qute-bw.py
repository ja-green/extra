#!/usr/bin/env python

# Copyright (C) 2022 Jack Green (jack@jackgreen.co)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

# file: bw.py
# date: 30/01/2022
# lang: python3

import argparse
import os
import subprocess
import sys
import tldextract
import time
import pyperclip

js_base = """
function canAccessIFrame(iframe) {
    var html = null;
    try {
      var innerDoc = iframe.contentDocument || iframe.contentWindow.document;
      html = innerDoc.body.innerHTML;
    } catch(err){
    }

    return(html !== null);
};
function isVisible(elem) {
    var style = elem.ownerDocument.defaultView.getComputedStyle(elem, null);
    if (style.getPropertyValue("visibility") !== "visible" ||
        style.getPropertyValue("display") === "none" ||
        style.getPropertyValue("opacity") === "0") {
        return false;
    }
    return elem.offsetWidth > 0 && elem.offsetHeight > 0;
};
function hasPasswordField(form) {
    var inputs = form.getElementsByTagName("input");
    for (var j = 0; j < inputs.length; j++) {
        var input = inputs[j];
        if (input.type == "password") {
            return true;
        }
    }
    return false;
};
function fillFieldsM1(form) {
    var inputs = form.getElementsByTagName("input");
    var visible_inputs = [];
    var username_input = null;
    var password_input = null;
    var i = 0;
    for (i = 0; i < inputs.length; i++) {
        if (isVisible(inputs[i])) {
            visible_inputs.push(inputs[i]);
        }
    }
    for (i = 0; i < visible_inputs.length; i++) {
        if (visible_inputs[i].type == "password") {
            password_input = visible_inputs[i];
            break;
        }
    }
    if (i != 0) {
        username_input = visible_inputs[i - 1];
    }
    if (username_input !== null && (username_input.type == "text" || username_input.type == "email")) {
        username_input.focus();
        username_input.value = %(username)s;
        username_input.dispatchEvent(new Event('change'));
        username_input.blur();
    }
    if (password_input !== null && (password_input.type == "password")) {
        password_input.focus();
        password_input.value = %(password)s;
        password_input.dispatchEvent(new Event('change'));
        password_input.blur();
    }
};
function fillFieldsM2(form) {
    var inputs = form.getElementsByTagName("input");
    var visible_inputs = [];
    var username_input = null;
    var i = 0;
    for (i = 0; i < inputs.length; i++) {
        if (isVisible(inputs[i])) {
            visible_inputs.push(inputs[i]);
        }
    }
    for (i = 0; i < visible_inputs.length; i++) {
        if ((visible_inputs[i].name.toLowerCase().includes("username") ||
        visible_inputs[i].name.toLowerCase().includes("email") ||
        visible_inputs[i].placeholder.toLowerCase().includes("username") ||
        visible_inputs[i].placeholder.toLowerCase().includes("email")) &&
        (visible_inputs[i].type == "text" || visible_inputs[i].type == "email")) {
            username_input = visible_inputs[i];
            break;
        }
    }
    if (username_input !== null && (username_input.type == "text" || username_input.type == "email")) {
        username_input.focus();
        username_input.value = %(username)s;
        username_input.dispatchEvent(new Event('change'));
        username_input.blur();
    }
};
var forms_list = document.getElementsByTagName("form");
var forms = Array.from(forms_list);
var iframes = document.getElementsByTagName("iframe");
var done = false;
for (var i = 0; i < iframes.length; i++) {
    if (canAccessIFrame(iframes[i])) {
        forms = forms.concat(Array.from(iframes[i].contentDocument.getElementsByTagName("form")));
    }
}
for (var i = 0; i < forms.length; i++) {
    if (hasPasswordField(forms[i])) {
        fillFieldsM1(forms[i]);
        done = true;
    }
}
if (!done) {
    for (var i = 0; i < forms.length; i++) {
        fillFieldsM2(forms[i]);
    }
}"""

def _qute_command(command):
    with open(os.environ['QUTE_FIFO'], 'w') as fifo:
        fifo.write(command + '\n')
        fifo.flush()

def _rbw_run(cmd) -> str:
    process = subprocess.run(
        ['rbw', 'unlocked'],
    )

    if process.returncode != 0:
        _qute_command("message-error \"qute-bw: vault locked; unlock and try again\"")
        sys.exit(0)

    process = subprocess.run(
        cmd,
        stdout = subprocess.PIPE,
        stderr = subprocess.PIPE,
    )

    err = process.stderr.decode('UTF-8').strip()
    if err:
        sys.exit(1)

    if process.returncode != 0:
        sys.exit(1)

    return process.stdout.decode('UTF-8').strip()

def fake_key(text, escape=True):
    sequence = ""

    if escape:
        for character in text:
            sequence += '" "' if character == ' ' else '\\%s' % (character)
    else:
        sequence = text

    _qute_command('fake-key %s' % (sequence))

def bw_get_all():
    return _rbw_run(['rbw', 'list']).splitlines()

def bw_get(name):
    out = {}
    out_lines = _rbw_run(['rbw', 'get', '--full', name]).splitlines()

    out['password'] = out_lines[0]
    try:
        out['username'] = out_lines[1].split("Username: ", 1)[1]
    except IndexError:
        out['username'] = None

    if 'TOTP Secret: ' in out_lines:
        out['totp'] = _rbw_run(['rbw', 'code', name])
    else:
        out['totp'] = None

    return out

def main():
    parser = argparse.ArgumentParser(allow_abbrev=False, add_help=False)
    parser.add_argument('to_enter', nargs='?', default='all')
    args = vars(parser.parse_args())

    url = os.getenv('QUTE_URL')
    if not url:
        return 1

    matched_candidates = []
    bw_names = bw_get_all()

    extract = tldextract.extract(url)
    attempted_candidates = []
    candidates = [
        extract.fqdn,
        extract.registered_domain,
        extract.subdomain + '.' + extract.domain,
        extract.domain,
        extract.ipv4
    ]

    for candidate in candidates:
        if candidate == None or candidate in attempted_candidates:
            continue

        attempted_candidates.append(candidate)
        for name in bw_names:
            if candidate.casefold() == name.casefold():
                matched_candidates.append(name)

    if len(matched_candidates) == 0:
        _qute_command("message-error \"qute-bw: no entries found\"")
        sys.exit(0)
    elif len(matched_candidates) == 1:
        selection = matched_candidates.pop()
    else:
        process = subprocess.run(
            ['dmenu'],
            input = '\n'.join(matched_candidates).encode('UTF-8'),
            stdout = subprocess.PIPE
        )

        if process.returncode != 0:
            sys.exit(1)
        selection = process.stdout.decode('UTF-8').strip()

    creds = bw_get(selection)

    js_command = js_base % {
        'username': repr(creds['username']),
        'password': repr(creds['password'])
    }
    _qute_command("jseval --quiet %s" % js_command.replace("\n", " "))

    if args['to_enter'] != 'totp' and creds['totp'] != None:
        pyperclip.copy(creds['totp'])
        _qute_command("message-info \"qute-bw: copied totp to clipboard; clearing after 30 secs\"")
        time.sleep(30)
        pyperclip.copy('')

    return 0

if __name__ == '__main__':
    sys.exit(main())
