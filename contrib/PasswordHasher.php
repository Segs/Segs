/*
 * File:        PasswordHasher.php
 * Date:        6/12/2018
 * Site:        http://www.segs.dev/
 * Authors:     Eric P. <erpmario@segs.dev> and Lloyd Dilley <lloyd@segs.dev>
 * Description: PasswordHasher.php is a PHP port of Projects/CoX/Common/PasswordHasher/PasswordHasher.cpp
 *              from the SEGS project. This program can be used to generate credentials based on user input
 *              via vars passed from a web client. The resultant hash and salt are in the same format as the
 *              output produced by the C++ counterpart. The credentials can be stored in the accounts table
 *              using any database implementation supported by SEGS.
 *
 * License:     Copyright (c) 2018 SEGS Project
 *              All rights reserved.
 *
 *              Redistribution and use in source and binary forms, with or without
 *              modification, are permitted provided that the following conditions are met:
 *                  * Redistributions of source code must retain the above copyright
 *                    notice, this list of conditions and the following disclaimer.
 *                  * Redistributions in binary form must reproduce the above copyright
 *                    notice, this list of conditions and the following disclaimer in the
 *                    documentation and/or other materials provided with the distribution.
 *                  * Neither the name of the copyright holders nor the names of its
 *                    contributors may be used to endorse or promote products derived
 *                    from this software without specific prior written permission.
 *
 *              THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *              ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *              WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *              DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
 *              FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *              DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *              SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *              CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *              OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *              OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/* 
    Example:
      1. Retrieve user and password supplied by web client preferably using HTTPS.
      2. Call the following to generate a salt: $sample_salt = generate_salt();
      3. Hash the combined password and salt: $hashed_pass_bytearr = hash_pass($password, $sample_salt);
      4. Insert $sample_salt and $hashed_pass_bytearr into the accounts table. Be sure to defend against
         SQL injections by escaping queries or using prepared statements.
*/

// Generate a salt (random string that gets appended to plaintext password) with a default length of 16
function generate_salt($salt_length = 16)
{
  $possible_chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012345678';
  $rand_string = '';
  for($i = 0; $i < $salt_length; ++$i)
  {
    $rand_string .= $possible_chars[random_int(0, strlen($possible_chars) - 1)];
  }
  return utf8_encode($rand_string);
}

// Return the binary representation of a SHA256-digested plaintext password + salt from generate_hash()
function hash_pass($plaintext_pass, $salt)
{
  return hash('sha256', $plaintext_pass . $salt, true);
}
