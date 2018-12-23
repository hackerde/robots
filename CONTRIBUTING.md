For contributing to this project, consider the following steps to ensure smooth, collaborative and bug-free development:

## To Add Code

1. Start off by creating a branch for the repository from the `master` branch.
Name it something appropriate.
For example, to add a feature where multiple robots can perform a task, a branch name like `multi-robot` would be suitable.
The command is: `git branch <branch_name>`

2. Now, switch to that branch: `git checkout <branch_name>`

3. Do all your file edits and changes while staying on that branch.

4. After testing that all changes are valid and the code compiles and runs, add, commit and push your changes to the branch:

  ```
  git add <file_1> <file_2> ...
  git commit -m "<message stating what your changes/files do>"
  git push origin <branch_name>
  ```

5. In `stogit` (the website), go into the repository.
In the menu on the left, click on *Merge Requests*.
Then, click on *New Merge Request*.
Select the *Source Branch* as your branch and *Target Branch* as `master`.
Click on *Compare Branches and Continue*.
On the next screen, give the merge request a tile (like what the merge request does).
If there are things that need further explanation, write it in the description.
Then click on *Submit Merge Request* and you are done!

## To Review Code

When someone creates a merge request, someone else has to review it and make sure it works before adding the changes in the master branch.

1. Start by pulling the branch on your machine: `git pull origin <branch_name>`

2. Compile and run the program to see it does both fine.

3. On `stogit`, if you click on the merge request and then click on the *Changes* tab, you will be able to see what exact changes were made.
Go through it and make sure everything looks good.

4. If everything turned out to be fine, then we are ready to add the changes to the `master` branch.
  
    i. Switch back to your master branch on your machine: `git checkout master`.
  
    ii. On `stogit`, go to the merge request and click *Merge* and you are done!

5. In case there is some issue, describe the issue by starting a discussion inside the merge request.

## Recommendations

* Perform regular commits.
Make one for every tiny change you make and not for big chunks.
They are cheap but super helpful for rolling back to older versions of code.

* Try not to edit anything on `stogit`.
Apart from merge requests, everything else should be done on your machine.

* If a branch has a major change, do not delete the branch after merging it.
You can delete it if the change is not that significant, but again, you do not have to.

* Changes can be undone, so do not panic!

* Try not to create branches of a branch.

* Always keep your `master` branch up to date with the `master` on the repository:
  ```
  git checkout master
  git pull origin master
  ```

* Do not merge your own merge request.
If you notice a merge request, give priority to that and start your own work after you have reviewed the code.

* If you edited a file on a wrong branch by mistake, try this:

  * Copy the file to a different location
  * `git checkout -- <file_name>` will download the version of the file on the repo
  * `git checkout <branch_name>` will switch to the branch you should be working on (or create one)
  * Copy the file back or modify it again (now in the correct branch)
  * Add, commit and push your changes.
Do not forget to create a merge request!

