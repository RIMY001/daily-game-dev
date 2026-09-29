using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;
using UnityEngine.InputSystem;

public class PlayerController : MonoBehaviour
{
    private enum Direction
    { 
        Up, Right, Left
    }
    private Rigidbody2D rb;
    private Animator anim;
    private SpriteRenderer sr;
    public float jumpDistance;
    private float moveDistance;
    private Vector2 destination;
    private Vector2 touchPosition;
    private Direction dir;

    private bool buttonHeld;
    private bool isJump;
    private bool canJump;

    private void Awake()
    {
        rb = GetComponent<Rigidbody2D>();
        anim = GetComponent<Animator>();
        sr = GetComponent<SpriteRenderer>();
    }

    private void Update()
    {
        if (canJump)
        {
            TriggerJump();     
        }
    }

    private void FixedUpdate()
    {
        if(isJump)
            rb.position = Vector2.Lerp(transform.position, destination, 0.134f);
    }

    private void OnTriggerStay2D(Collider2D other)
    {
        if (other.CompareTag("Border") || other.CompareTag("Car"))
        {
            
        }

        if (!isJump && other.CompareTag("Obstacle"))
        {

        }
    }

    #region INPUT 输入回调函数
    public void Jump(InputAction.CallbackContext context)
    {
        //TODO:执行跳跃，跳跃的距离，记录分数，播放跳跃的音效
        if (context.phase == InputActionPhase.Performed && !isJump)
        {
            moveDistance = jumpDistance;
            //Debug.Log("JUMP!" +" "+ moveDistance);
            //执行跳跃
            
            canJump = true;
        }
    }

    public void LongJump(InputAction.CallbackContext context)
    {
        if (context.performed && !isJump)
        {
            moveDistance = jumpDistance * 2;
            buttonHeld = true;
        }

        if (context.canceled && buttonHeld && !isJump)
        {
            //执行跳跃
            //Debug.Log("LONG JUMP!" + " " + moveDistance);
            buttonHeld = false;
            
            canJump = true;
        }
    }

    public void GetTouchPosition(InputAction.CallbackContext context)
    {
        if (context.performed)
        {
            //Debug.Log(context.ReadValue<Vector2>());
            touchPosition = Camera.main.ScreenToWorldPoint(context.ReadValue<Vector2>());
            //Debug.Log(touchPosition);
            var offset = ((Vector3)touchPosition - transform.position).normalized;
            if (Mathf.Abs(offset.x) <= 0.7f)
            {
                dir = Direction.Up;
            }
            else if (offset.x < 0)
            {
                dir = Direction.Left;
            }
            else if (offset.x > 0)
            {
                dir = Direction.Right;
            }
        }
    }

    #endregion

    /// <summary>
    /// 触发执行跳跃动画
    /// </summary>
    private void TriggerJump()
    {
        // 获得移动方向，播放动画
        canJump = false;
        switch (dir)
        {
            //  触发切换左右方向动画
            case Direction.Up:
                anim.SetBool("isSide", false);
                destination = new Vector2(transform.position.x, transform.position.y + moveDistance);
                transform.localScale = Vector3.one;
                break;
            case Direction.Right:
                anim.SetBool("isSide", true);
                destination = new Vector2(transform.position.x + moveDistance, transform.position.y);
                transform.localScale = new Vector3(-1,1,1);
                break;
            case Direction.Left:
                anim.SetBool("isSide", true);
                destination = new Vector2(transform.position.x - moveDistance, transform.position.y);
                transform.localScale = Vector3.one;
                break;
        }
        anim.SetTrigger("Jump");
    }

    #region Animation Even
    public void JumpAnimationEven()
    {
        //改变状态
        isJump = true;

        //修改排序图层
        sr.sortingLayerName = "Front";
        
    }

    public void FinishJumpAnimationEven()
    {
        isJump = false;
        //修改排序图层
        sr.sortingLayerName = "Middle";
    }

    #endregion
}
