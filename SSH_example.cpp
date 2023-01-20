#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>
 
int main()
{
  ssh_session my_ssh_session;
  ssh_channel channel;
  int rc;
  int nbytes;
  char *password;
  char buffer[256];
  
  ///// Open session and set options /////
  my_ssh_session = ssh_new();
  if (my_ssh_session == NULL)
    exit(-1);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "192.168.0.39");
  ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "user");
  
  // Connect to server
  rc = ssh_connect(my_ssh_session);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error connecting to localhost: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_free(my_ssh_session);
    exit(-1);
  }
 
  // Verify the server's identity
  // For the source code of verify_knownhost(), check previous example
  // if (verify_knownhost(my_ssh_session) < 0)
  // {
  //   ssh_disconnect(my_ssh_session);
  //   ssh_free(my_ssh_session);
  //   exit(-1);
  // }
 
  ///// Authenticate ourselves /////
  // password = getpass("Password: ");
  password = (char *)"nrmk2013";
  rc = ssh_userauth_password(my_ssh_session, NULL, password);
  if (rc != SSH_AUTH_SUCCESS)
  {
    fprintf(stderr, "Error authenticating with password: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    exit(-1);
  }

  ///// Make Channel /////
  channel = ssh_channel_new(my_ssh_session);
  if (channel == NULL) return SSH_ERROR;
 
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return rc;
  }

  ///// Request_exec /////
  rc = ssh_channel_request_exec(channel, "ls -l");
  if (rc != SSH_OK)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }

  ///// Read from channel  /////
  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  while (nbytes > 0)
  {
    if (fwrite(buffer, 1, nbytes, stdout) != nbytes)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  }
   
  if (nbytes < 0)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_ERROR;
  }
  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);

  ssh_disconnect(my_ssh_session);
  ssh_free(my_ssh_session);
}
